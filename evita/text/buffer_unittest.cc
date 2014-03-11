// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/css/style.h"
#include "evita/text/buffer.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/text/range.h"

namespace {

class BufferTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: BufferTest()
      : buffer_(new text::Buffer(L"*test*", new text::PlainTextMode())) {
  }
  public: virtual ~BufferTest() {
  }

  public: text::Buffer* buffer() const { return buffer_.get(); }

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

TEST_F(BufferTest, InsertAt) {
  buffer()->InsertBefore(0, L"abc");

  css::Style style_values1;;
  style_values1.set_syntax(1);
  buffer()->SetStyle(1, 2, style_values1);

  css::Style style_values2;;
  style_values2.set_syntax(2);
  buffer()->SetStyle(2, 3, style_values2);

  auto range = std::make_unique<text::Range>(buffer(), 2, 2);

  // Insert 'X' at 'c'.
  buffer()->Insert(2, L"X");
  EXPECT_EQ(4, buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(2));
  EXPECT_EQ('c', buffer()->GetCharAt(3));
  EXPECT_EQ(2, buffer()->GetStyleAt(2).syntax()) <<
      "The style of inserted text is the style of insertion position.";
  EXPECT_EQ(2, buffer()->GetStyleAt(3).syntax()) <<
      "The style at insertion position isn't changed.";
  EXPECT_EQ(2, range->GetStart()) <<
    "The range at insertion position should not be moved.";
  EXPECT_EQ(2, range->GetEnd()) <<
    "The range at insertion position should not be moved.";
}

TEST_F(BufferTest, InsertBefore) {
  buffer()->InsertBefore(0, L"abc");

  css::Style style_values1;;
  style_values1.set_syntax(1);
  buffer()->SetStyle(1, 2, style_values1);

  css::Style style_values2;;
  style_values2.set_syntax(2);
  buffer()->SetStyle(2, 3, style_values2);

  auto range = std::make_unique<text::Range>(buffer(), 2, 2);

  // Insert 'X' before 'c'.
  buffer()->InsertBefore(2, L"X");
  EXPECT_EQ(4, buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(2));
  EXPECT_EQ('c', buffer()->GetCharAt(3));
  EXPECT_EQ(1, buffer()->GetStyleAt(2).syntax()) <<
      "The style of inserted text is inherited from styles before insertion"
      " position.";
  EXPECT_EQ(2, buffer()->GetStyleAt(3).syntax()) <<
      "The style at insertion position isn't changed.";
  EXPECT_EQ(3, range->GetStart()) <<
    "The range at insertion position should be push back.";
  EXPECT_EQ(3, range->GetEnd()) <<
    "The range at insertion position should be push back.";
}

TEST_F(BufferTest, SetStyle) {
  buffer()->InsertBefore(0, L"foo bar baz");

  // Set all text font size to 30.
  css::Style style_font_size_30;
  style_font_size_30.set_font_size(30);
  buffer()->SetStyle(0, buffer()->GetEnd(), style_font_size_30);

  // Color "bar" to red.
  css::Style style_color_red;
  style_color_red.set_color(css::Color(0xCC, 0, 0));
  buffer()->SetStyle(4, 7, style_color_red);

  EXPECT_EQ(30, buffer()->GetStyleAt(4).font_size()) <<
      "Set color doesn't affect font size.";
}

}  // namespace
