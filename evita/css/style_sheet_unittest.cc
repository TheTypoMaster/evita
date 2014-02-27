// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "base/logging.h"
#include "evita/css/style_selector.h"
#include "evita/css/style_sheet.h"

namespace {

using css::Color;
using css::Style;
using css::StyleSelector;
using css::StyleSheet;

class StyleSheetTest : public ::testing::Test {
  public: StyleSheetTest() = default;
  public: ~StyleSheetTest() = default;

  DISALLOW_COPY_AND_ASSIGN(StyleSheetTest);
};

TEST_F(StyleSheetTest, Resolve_default) {
  StyleSheet style_sheet;
  const auto& style1 = style_sheet.Resolve(StyleSelector::defaults());
  EXPECT_EQ(css::FontStyle::Normal, style1.font_style());
  EXPECT_EQ(css::FontWeight::Normal, style1.font_weight());
}

TEST_F(StyleSheetTest, Resolve_some) {
  StyleSheet style_sheet;
  style_sheet.AddRule(StyleSelector::active_selection(),
                      Style(Color(1, 2, 3), Color(4, 5, 6)));
  const auto& style1 = style_sheet.Resolve(StyleSelector::active_selection());
  EXPECT_EQ(Color(1, 2, 3), style1.color());
  EXPECT_EQ(Color(4, 5, 6), style1.bgcolor());
  EXPECT_EQ(css::FontStyle::Normal, style1.font_style());
}

}  // namespace
