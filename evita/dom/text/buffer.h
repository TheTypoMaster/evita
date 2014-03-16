//////////////////////////////////////////////////////////////////////////////
//
// Editor - Buffer
// listener/winapp/ed_Buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Buffer.h#1 $
//
#if !defined(INCLUDE_evita_dom_buffer_h)
#define INCLUDE_evita_dom_buffer_h

#include "evita/dom/view_delegate.h"
#include "evita/text/buffer.h"

namespace dom {

class Buffer : public text::Buffer, public DoubleLinkedNode_<Buffer> {
  public: Buffer(const base::string16& name, text::Mode* mode);
  public: virtual ~Buffer();

  public: void set_filename(const base::string16& filename) {
    filename_ = filename;
  }

  // [F]
  public: void FinishIo(uint);

  // [L]
  public: bool Load(const base::string16& filename,
                    const dom::ViewDelegate::LoadFileCallback& callback);

  // [S]
  public: bool Save(const base::string16& filename,
                    int code_page, NewlineMode newline_mode,
                    const dom::ViewDelegate::SaveFileCallback& callback);

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

} // namespace dom

using Buffer = dom::Buffer;

#endif //!defined(INCLUDE_evita_dom_buffer_h)