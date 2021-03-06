// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_focus_event_h)
#define INCLUDE_evita_dom_events_focus_event_h

#include "evita/dom/events/ui_event.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class FocusEventInit;
class Window;

namespace bindings {
class FocusEventClass;
}

class FocusEvent : public v8_glue::Scriptable<FocusEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(FocusEvent)
  friend class bindings::FocusEventClass;

  private: gc::Member<EventTarget> related_target_;

  public: FocusEvent(const base::string16& type,
                     const FocusEventInit& init_dict);
  private: FocusEvent(const base::string16& type);
  public: virtual ~FocusEvent();

  private: EventTarget* related_target() const {
    return related_target_.get();
  }

  DISALLOW_COPY_AND_ASSIGN(FocusEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_focus_event_h)
