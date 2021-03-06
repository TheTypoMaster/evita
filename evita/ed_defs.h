//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_defs.h#2 $
//
#ifndef EVITA_ED_DEFS_H_
#define EVITA_ED_DEFS_H_

#include "base/strings/string16.h"

namespace text {

typedef int Count;
typedef int Posn;
const Posn Posn_Max = 1 << 28;

class Buffer;
class Range;
class Interval;

}  // namespace text

enum {
  Count_Backward = -1 << 28,
  Count_Forward = +1 << 28,
  Count_Max = Count_Forward,
};

//////////////////////////////////////////////////////////////////////
//
// Unit
//
enum Unit {
  Unit_Buffer,
  Unit_Char,
  Unit_Line,
  Unit_Page,
  Unit_Paragraph,
  Unit_Screen,  // valid only for selection
  Unit_Sentence,
  Unit_Window,      // valid only for selection
  Unit_WindowLine,  // valid only for selection
  Unit_Word,
};

#endif  // EVITA_ED_DEFS_H_
