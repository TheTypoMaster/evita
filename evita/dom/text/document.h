// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_document_h)
#define INCLUDE_evita_dom_document_h

#include <memory>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/dom/events/event_target.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace text {
class Buffer;
struct LineAndColumn;
}

namespace dom {

class Range;
class RegExp;

namespace bindings {
class DocumentClass;
}

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document : public v8_glue::Scriptable<Document, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Document);
  friend class bindings::DocumentClass;

  private: std::unique_ptr<text::Buffer> buffer_;
  private: base::Time last_write_time_;
  private: base::string16 file_name_;
  // TODO(yosi) When we set |properties| with |v8::Object::Set()|, it doesn't
  // appeared in JavaScript. I'm not sure why. So, we hold |properties| in
  // C++.
  private: v8_glue::ScopedPersistent<v8::Object> properties_;

  private: explicit Document(const base::string16& name);
  public: virtual ~Document();

  public: const text::Buffer* buffer() const { return buffer_.get(); }
  public: text::Buffer* buffer() { return buffer_.get(); }
  public: base::char16 charCodeAt(text::Posn position) const;
  public: const base::string16& file_name() const;
  public: void set_file_name(const base::string16& file_name);
  public: base::Time last_write_time() const;
  public: void set_last_write_time(base::Time last_write_time);
  public: text::Posn length() const;
  public: bool modified() const;
  public: void set_modified(bool new_modified);
  public: const base::string16& name() const;
  public: v8::Handle<v8::Object> properties() const;
  public: bool read_only() const;
  public: void set_read_only(bool read_only) const;
  public: int spelling_at(text::Posn offset) const;
  // Returns syntax at |ofset|.
  public: const base::string16& syntax_at(text::Posn offset) const;
  public: v8::Handle<v8::Object> style_at(text::Posn position) const;

  public: bool CheckCanChange() const;
  public: void ClearUndo();
  public: void EndUndoGroup(const base::string16& name);
  public: text::LineAndColumn GetLineAndColumn(text::Posn offset) const;
  public: bool IsValidPosition(text::Posn position) const;
  public: v8::Handle<v8::Value> Match(RegExp* regexp, int start, int end);
  public: static Document* New(const base::string16& name);
  public: Posn Redo(Posn position);
  public: void RenameTo(const base::string16& new_name);
  public: base::string16 Slice(int start, v8_glue::Optional<int> opt_end);
  public: void StartUndoGroup(const base::string16& name);
  public: Posn Undo(Posn position);

  DISALLOW_COPY_AND_ASSIGN(Document);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_document_h)
