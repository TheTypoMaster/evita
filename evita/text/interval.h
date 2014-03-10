// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_interval_h)
#define INCLUDE_evita_text_interval_h

#include "evita/ed_BinTree.h"
#include "evita/css/style.h"
#include "evita/li_util.h"

namespace text {

class IntervalSet;

class Interval : public BinaryTree<Interval>::NodeBase,
                 public DoubleLinkedNode_<Interval> {
  friend class Buffer;
  friend class IntervalSet;

  private: Posn m_lEnd;
  private: Posn m_lStart;
  private: int m_nZ;
  private: css::Style m_Style;

  public: Interval(Posn lStart, Posn lEnd, int nZ = 9);
  public: ~Interval();

  // [C]
  public: bool CanMerge(const Interval*) const;

  public: int Compare(const Interval* pThat) const {
    return m_lStart - pThat->m_lStart;
  }

  public: bool Contains(Posn lPosn) const {
    return lPosn >= m_lStart && lPosn < m_lEnd;
  }

  // [G]
  public: Posn GetEnd() const { return m_lEnd; }
  public: Posn GetStart() const { return m_lStart; }
  public: const css::Style& GetStyle() const { return m_Style; }

  // [I]
  public: bool IsEmpty() const { return m_lStart == m_lEnd; }

  // [S]
  public: void SetStyle(const css::Style&);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_interval_h)
