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

#include "evita/text/buffer.h"

#include <memory>

namespace Command {
class KeyBindEntry;
class KeyBinds;
}

namespace dom {

class Buffer : public text::Buffer, public DoubleLinkedNode_<Buffer> {
  private: std::unique_ptr<Command::KeyBinds> key_bindings_;

  public: Buffer(const base::string16& name, text::Mode* pMode = nullptr);
  public: virtual ~Buffer();

  // [B]
  public: void BindKey(uint key_code, Command::KeyBindEntry* entry);

  // [F]
  public: void FinishIo(uint);

  // [L]
  public: bool Load(const char16*);

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint key_code) const;

  // [O]
  public: bool OnIdle(uint);

  // [R]
  public: bool Reload();

  // [S]
  public: bool Save(const char16*, uint, NewlineMode);

  // [U]
  public: void UpdateFileStatus(bool = false);
};

} // namespace dom

using Buffer = dom::Buffer;

#endif //!defined(INCLUDE_evita_dom_buffer_h)
