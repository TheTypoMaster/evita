// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/dom/buffer.h"

#include "evita/text/modes/mode.h"

#include "evita/editor/application.h"
#include "evita/resource.h"
#include "evita/vi_Frame.h"

namespace dom {

Buffer::Buffer(const base::string16& name, text::Mode* mode)
    : text::Buffer(name, mode) {
}

Buffer::~Buffer() {
}

bool Buffer::OnIdle(uint) {
  #if DEBUG_STYLE
  {
    DEBUG_PRINTF(L"%p\n", this);
    int nNth = 0;
    foreach (EnumInterval, oEnum, this) {
      auto const pIntv = oEnum.Get();
      DEBUG_PRINTF(L"%d [%d, %d] #%06X\n",  nNth, pIntv->GetStart(),
          pIntv->GetEnd(), pIntv->GetStyle()->GetColor());
      nNth += 1;
    }
  }
  #endif

  // Note: If we decrase number for coloring amount, we'll see
  // more redisplay.
  return GetMode()->DoColor(500);
}

void Buffer::FinishIo(uint const) {
  m_eState = State_Ready;
}

}  // namespace dom
