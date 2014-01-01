#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Edit Buffer
// listener/winapp/vi_buffer.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Buffer.cpp#1 $
//
#include "evita/dom/document.h"

#include "./ed_Mode.h"

#include "evita/editor/application.h"
#include "./vi_Frame.h"

namespace dom {

Document::Document(const char16* pwsz, Edit::Mode* pMode)
    : Edit::Buffer(pwsz, pMode) {
}

Document::~Document() {
}

// Returns true if buffer is safe to kill.
bool Document::CanKill() {
  if (!NeedSave())
    return true;

  char16 wsz[1024];
  ::wsprintf(wsz,
      L"Do you want to save the changes to %s?",
      GetName() );

  auto const pFrame = Application::instance()->GetActiveFrame();

  auto const iAnswer = Application::instance()->Ask(
      MB_ICONWARNING | MB_YESNOCANCEL,
      IDS_ASK_SAVE,
      GetName() );

  switch (iAnswer) {
    case IDCANCEL:
      return false;

    case IDYES:
      if (!Application::instance()->SaveBuffer(pFrame, this))
        return false;
      break;
  }

  return true;
}

// Returns MRU window
Document::Window* Document::GetWindow() const {
  const auto* mru = m_oWindows.GetFirst();
  for (auto& window: m_oWindows) {
    if (mru->GetActiveTick() < window.GetActiveTick())
      mru = &window;
  }
  return const_cast<Window*>(mru);
}

bool Document::OnIdle(uint) {
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

}  // namespace dom
