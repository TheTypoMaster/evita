// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_selection_h)
#define INCLUDE_evita_dom_selection_h

#include "evita/dom/text/document.h"
#include "evita/dom/windows/document_window.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Document;
class DocumentWindow;

namespace bindings {
class SelectionClass;
}

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection : public v8_glue::Scriptable<Selection> {
  DECLARE_SCRIPTABLE_OBJECT(Selection);
  friend class bindings::SelectionClass;

  private: gc::Member<Document> document_;
  private: gc::Member<DocumentWindow> document_window_;

  protected: Selection(DocumentWindow* document_window, Document* document);
  protected: virtual ~Selection();

  public: Document* document() const { return document_; }
  protected: DocumentWindow* window() const { return document_window_; }

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_selection_h)
