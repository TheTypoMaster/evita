// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/ui_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/ui_event_init.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom   {
namespace {
//////////////////////////////////////////////////////////////////////
//
// UiEventClass
//
class UiEventClass :
    public v8_glue::DerivedWrapperInfo<UiEvent, Event> {

  public: explicit UiEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~UiEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &UiEventClass::NewUiEvent);
  }

  private: static UiEvent* NewUiEvent(const base::string16& type,
      v8_glue::Optional<v8::Handle<v8::Object>> opt_dict) {
    UiEventInit init_dict;
    if (!init_dict.Init(opt_dict.value))
      return nullptr;
    return new UiEvent(type, init_dict);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("detail", &UiEvent::detail)
        .SetProperty("view", &UiEvent::view);
  }

  DISALLOW_COPY_AND_ASSIGN(UiEventClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// UiEvent
//
DEFINE_SCRIPTABLE_OBJECT(UiEvent, UiEventClass);

UiEvent::UiEvent(const base::string16& type,
                 const UiEventInit& init_dict)
    : ScriptableBase(type, init_dict), detail_(init_dict.detail()),
      view_(init_dict.view()) {
}

UiEvent::~UiEvent() {
}

}  // namespace dom
