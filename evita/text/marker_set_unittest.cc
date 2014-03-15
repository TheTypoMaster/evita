// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/marker_set.h"

namespace {

using text::BufferMutationObserver;
using text::Marker;
using text::MarkerSet;

class MockBufferMutationObservee : public text::BufferMutationObservee {
  private: virtual void AddObserver(BufferMutationObserver*) override {}
  private: virtual void RemoveObserver(BufferMutationObserver*) override {}
};

class MarkerSetTest : public ::testing::Test {
  protected: enum Spelling {
    None,
    Correct,
    Misspelled,
  };

  private: MockBufferMutationObservee mutation_observee_;
  private: MarkerSet marker_set_;

  protected: MarkerSetTest() : marker_set_(&mutation_observee_) {
  }
  public: virtual ~MarkerSetTest() {
  }

  protected: MarkerSet* marker_set() { return &marker_set_; }

  protected: void RemoveMarker(text::Posn start, text::Posn end) {
    marker_set()->RemoveMarker(start, end);
  }

  protected: Marker GetAt(Posn offset) {
    auto const marker = marker_set_.GetMarkerAt(offset);
    return marker ? *marker : Marker();
  }

  protected: void InsertMarker(text::Posn start, text::Posn end, int type) {
    marker_set()->InsertMarker(start, end, type);
  }

  DISALLOW_COPY_AND_ASSIGN(MarkerSetTest);
};

TEST_F(MarkerSetTest, DeleteMarker_cover) {
  // before: --CC--
  // insert: -____--
  // after:  -____--
  InsertMarker(150, 250, Correct);
  RemoveMarker(100, 300);
  EXPECT_EQ(Marker(), GetAt(100));
  EXPECT_EQ(Marker(), GetAt(125));
  EXPECT_EQ(Marker(), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(225));
  EXPECT_EQ(Marker(), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_cross_left) {
  // before: ---CC--
  // insert: --__---
  // after:  --__C--
  InsertMarker(200, 300, Correct);
  RemoveMarker(150, 250);
  EXPECT_EQ(Marker(), GetAt(150));
  EXPECT_EQ(Marker(250, 300, Correct), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_cross_right) {
  // before: --CCCC----
  // insert: ----____--
  // after:  --CC__----
  InsertMarker(100, 250, Correct);
  RemoveMarker(200, 300);
  EXPECT_EQ(Marker(100, 200, Correct), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(200));
  EXPECT_EQ(Marker(), GetAt(250));
}

TEST_F(MarkerSetTest, DeleteMarker_disjoint) {
  // before: --CC--
  // insert: ------__--
  // after:  --CC------
  InsertMarker(100, 200, Correct);
  RemoveMarker(300, 400);
  EXPECT_EQ(Marker(100, 200, Correct), GetAt(150));
  EXPECT_EQ(Marker(), GetAt(350));
}

TEST_F(MarkerSetTest, DeleteMarker_same) {
  InsertMarker(100, 200, Correct);
  RemoveMarker(100, 200);
  auto const marker = marker_set()->GetLowerBoundMarker(100);
  EXPECT_EQ(Marker(), marker ? *marker : Marker());
}

TEST_F(MarkerSetTest, DeleteMarker_split) {
  // before: -CCCCCC--
  // insert: ---__--
  // after:  -CC__CC--
  InsertMarker(200, 400, Correct);
  RemoveMarker(250, 350);
  EXPECT_EQ(Marker(), GetAt(199));
  EXPECT_EQ(Marker(200, 250, Correct), GetAt(200));
  EXPECT_EQ(Marker(200, 250, Correct), GetAt(225));
  EXPECT_EQ(Marker(), GetAt(300));
  EXPECT_EQ(Marker(), GetAt(349));
  EXPECT_EQ(Marker(350, 400, Correct), GetAt(350));
  EXPECT_EQ(Marker(350, 400, Correct), GetAt(399));
  EXPECT_EQ(Marker(), GetAt(400));
}

TEST_F(MarkerSetTest, GetMarkerAt) {
  InsertMarker(5, 10, Correct);
  EXPECT_EQ(Marker(), GetAt(0));
  EXPECT_EQ(Marker(), GetAt(4));
  EXPECT_EQ(Marker(5, 10, Correct), GetAt(5));
  EXPECT_EQ(Marker(5, 10, Correct), GetAt(9));
  EXPECT_EQ(Marker(), GetAt(10));
  EXPECT_EQ(Marker(), GetAt(11));
}

TEST_F(MarkerSetTest, GetLowerBoundMarker) {
  InsertMarker(5, 10, Correct);
  EXPECT_EQ(Marker(), GetAt(0));
  EXPECT_EQ(Marker(), GetAt(4));
  EXPECT_EQ(Marker(5, 10, Correct), GetAt(5));
  EXPECT_EQ(Marker(5, 10, Correct), GetAt(9));
  EXPECT_EQ(Marker(), GetAt(10));
}

TEST_F(MarkerSetTest, InsertMarker_cover) {
  // before: --CC--
  // insert: -MMMM--
  // after:  -MMMM--
  InsertMarker(150, 250, Correct);
  InsertMarker(100, 300, Misspelled);
  EXPECT_EQ(Marker(100, 300, Misspelled), GetAt(100));
  EXPECT_EQ(Marker(100, 300, Misspelled), GetAt(125));
  EXPECT_EQ(Marker(100, 300, Misspelled), GetAt(150));
  EXPECT_EQ(Marker(100, 300, Misspelled), GetAt(225));
  EXPECT_EQ(Marker(100, 300, Misspelled), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_left) {
  // before: ---MM--
  // insert: --CC---
  // after:  --CCM--
  InsertMarker(200, 300, Correct);
  InsertMarker(150, 250, Misspelled);
  EXPECT_EQ(Marker(150, 250, Misspelled), GetAt(150));
  EXPECT_EQ(Marker(250, 300, Correct), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_cross_right) {
  // before: --MMMM----
  // insert: ----CCCC--
  // after:  --MMCCCC--
  InsertMarker(100, 250, Misspelled);
  InsertMarker(200, 300, Correct);
  EXPECT_EQ(Marker(100, 200, Misspelled), GetAt(150));
  EXPECT_EQ(Marker(200, 300, Correct), GetAt(200));
  EXPECT_EQ(Marker(200, 300, Correct), GetAt(250));
}

TEST_F(MarkerSetTest, InsertMarker_disjoint) {
  // before: --CC--
  // insert: ------CC--
  // after:  --CC--CC--
  InsertMarker(100, 200, Correct);
  InsertMarker(300, 400, Correct);
  EXPECT_EQ(Marker(100, 200, Correct), GetAt(150));
  EXPECT_EQ(Marker(300, 400, Correct), GetAt(350));
}

TEST_F(MarkerSetTest, InsertMarker_merge) {
  // before: --CC--CC--CC--
  // insert: --MMMMMMMMMM--
  // after:  --MMMMMMMMMM--
  InsertMarker(10, 20, Correct);
  InsertMarker(40, 60, Correct);
  InsertMarker(80, 90, Correct);
  InsertMarker(10, 90, Misspelled);
  EXPECT_EQ(Marker(10, 90, Misspelled), GetAt(10));
  EXPECT_EQ(Marker(10, 90, Misspelled), GetAt(20));
  EXPECT_EQ(Marker(10, 90, Misspelled), GetAt(30));
  EXPECT_EQ(Marker(10, 90, Misspelled), GetAt(89));
  EXPECT_EQ(Marker(), GetAt(90));
}

TEST_F(MarkerSetTest, InsertMarker_split) {
  // before: --CCCCCC--
  // insert: ----MM----
  // after:  --CCMMCC--
  InsertMarker(200, 400, Correct);
  InsertMarker(250, 350, Misspelled);
  EXPECT_EQ(Marker(), GetAt(199));
  EXPECT_EQ(Marker(200, 250, Correct), GetAt(200));
  EXPECT_EQ(Marker(200, 250, Correct), GetAt(225));
  EXPECT_EQ(Marker(250, 350, Misspelled), GetAt(300));
  EXPECT_EQ(Marker(250, 350, Misspelled), GetAt(349));
  EXPECT_EQ(Marker(350, 400, Correct), GetAt(350));
  EXPECT_EQ(Marker(350, 400, Correct), GetAt(399));
  EXPECT_EQ(Marker(), GetAt(400));
}

}  // namespace