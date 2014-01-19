//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - Haskell Mode
// listener/winapp/mode_Haskell.h
//
// Copyright (C) 1996-2009 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Haskell.h#1 $
//
#if !defined(INCLUDE_mode_Haskell_h)
#define INCLUDE_mode_Haskell_h

#include "evita/ed_Mode.h"

#include "evita/text/buffer.h"

namespace text
{

/// <summary>
///  Haskell mode factory
/// </summary>
class HaskellModeFactory : public ModeFactory
{
    // ctor
    public: HaskellModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"hs hsc"; }

    public: virtual const char16* GetName() const override
        { return L"Haskell"; }
}; // HaskellModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_Haskell_h)
