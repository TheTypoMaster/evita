// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/mason_mode.h"

#include "evita/dom/modes/mode_factory.h"

namespace text
{
/// <summary>
///   Returns true if Mason mode support specified buffer name.
/// </summary>
bool MasonModeFactory::IsSupported(const char16* pwsz) const
{
    if (0 == lstrcmpW(pwsz, L"autohandler")) return true;
    if (0 == lstrcmpW(pwsz, L"dhandler")) return true;
    return ModeFactory::IsSupported(pwsz);
} // MasonModeFactory::IsSupported

}  // namespace text
