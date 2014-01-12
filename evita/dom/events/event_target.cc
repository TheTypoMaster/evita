// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_target.h"

#include <list>
#include <unordered_map>

#include "common/adoptors/reverse.h"
#include "evita/dom/events/event.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// EventTargeClass
//
class EventTargeClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: EventTargeClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~EventTargeClass() = default;

  private: static EventTarget* NewEventTarget() {
    ScriptController::instance()->ThrowError("Can't create EventTarget.");
    return nullptr;
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    BaseClass::SetupInstanceTemplate(builder);
    builder
      .SetMethod("addEventListener", &EventTarget::AddEventListener)
      .SetMethod("dispatchEvent", &EventTarget::DispatchEvent)
      .SetMethod("removeEventListener", &EventTarget::RemoveEventListener);
  }
};

v8::Handle<v8::Object> GetCallee(v8::Isolate* isolate,
                                 v8::Handle<v8::Value> object) {
  if (!object->IsObject())
    return v8::Handle<v8::Object>();
  if (object->ToObject()->IsCallable())
    return object->ToObject();
  auto handler = object->ToObject()->Get(
      gin::StringToV8(isolate, "handleEvent"));
  if (handler.IsEmpty() || !handler->IsObject())
    return v8::Handle<v8::Object>();
  return handler->ToObject();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EventTarget::EventListenerMap
//
class EventTarget::EventListenerMap {
  private: struct EventListener {
    bool capture;
    v8_glue::ScopedPersistent<v8::Object> callback;

    EventListener(v8::Isolate* isoalte, v8::Handle<v8::Object> callback,
                  bool capture)
        : capture(capture), callback(isoalte, callback) {
    }
  };
  public: typedef std::list<EventListener*> EventListenerList;

  private: std::unordered_map<base::string16, EventListenerList*> map_;

  public: EventListenerMap() = default;
  public: ~EventListenerMap() = default;

  public: void Add(const base::string16& type,
                   v8::Handle<v8::Object> callback,
                   bool capture) {
    auto isolate = v8::Isolate::GetCurrent();
    if (auto const list = Find(type)) {
      for (auto runner : *list) {
        if (runner->callback == callback && runner->capture == capture) {
          runner->callback.Reset(isolate, callback);
          return;
        }
      }
      list->push_back(new EventListener(isolate, callback, capture));
      return;
    }

    auto list = new EventListenerList();
    list->push_back(new EventListener(isolate, callback, capture));
    map_[type] = list;
  }

  public: EventListenerList* Find(const base::string16& type) {
    auto present = map_.find(type);
    return present == map_.end() ? nullptr : present->second;
  }

  public: void Remove(const base::string16& type,
                      v8::Handle<v8::Object> callback,
                      bool capture) {
    auto list = Find(type);
    if (!list)
      return;
    for (auto it = list->begin(); it != list->end(); ++it) {
      if ((*it)->callback == callback && (*it)->capture == capture) {
        list->erase(it);
        return;
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
DEFINE_SCRIPTABLE_OBJECT(EventTarget, EventTargeClass);

EventTarget::EventTarget()
    : event_listener_map_(new EventListenerMap()) {
}

EventTarget::~EventTarget() {
}

void EventTarget::AddEventListener(const base::string16& type,
                            v8::Handle<v8::Object> listener,
                            Optional<bool> capture) {
  event_listener_map_->Add(type, listener, capture.get(false));
}

EventTarget::EventPath EventTarget::BuildEventPath() const {
  return std::vector<EventTarget*>();
}

bool EventTarget::DispatchEvent(Event* event) {
  if (event->event_phase() != Event::kNone || event->type().empty()) {
    ScriptController::instance()->ThrowError("EventTarget.dispatchEvent: "
        "InvalidStartError");
    return false;
  }

  Event::DispatchScope dispatch_scope(event, this);

  auto event_path = BuildEventPath();
  for (auto target : event_path) {
    DCHECK(target != this);
    dispatch_scope.set_current_target(target);
    target->InvokeEventListeners(event);
    if (event->stop_propagation())
      break;
  }

  dispatch_scope.StartAtTarget();
  dispatch_scope.set_current_target(this);
  InvokeEventListeners(event);

  if (event->bubbles()) {
    dispatch_scope.StartBubbling();
    for (auto target : common::adoptors::reverse(event_path)) {
      DCHECK(target != this);
      if (event->stop_propagation())
        break;
      dispatch_scope.set_current_target(target);
      target->InvokeEventListeners(event);
    }
  }
  return !event->default_prevented();
}

void EventTarget::InvokeEventListeners(Event* event) {
  auto listeners = event_listener_map_->Find(event->type());
  if (!listeners)
    return;
  auto listeners_copy = *listeners;
  for (auto listener : listeners_copy) {
    if (event->stop_immediate_propagation())
      return;
    if (event->event_phase() == Event::kCapturingPhase) {
      if (!listener->capture)
        continue;
    } else if (event->event_phase() == Event::kBubblingPhase) {
      if (listener->capture)
        continue;
    }

    auto const isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Handle<v8::Value> argv[1] {event->GetWrapper(isolate)};
    auto const callee = GetCallee(isolate,
        listener->callback.NewLocal(isolate));
    if (callee.IsEmpty())
      continue;
    v8::TryCatch try_catch;
    auto const value = callee->CallAsFunction(GetWrapper(isolate), 1, argv);
    if (value.IsEmpty())
        ScriptController::instance()->LogException(try_catch);
  }
}

void EventTarget::RemoveEventListener(const base::string16& type,
                                      v8::Handle<v8::Object> listener,
                                      Optional<bool> capture) {
  event_listener_map_->Remove(type, listener, capture.get(false));
}

}  // namespace dom
