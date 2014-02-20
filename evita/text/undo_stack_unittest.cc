// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/buffer.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/text/range.h"

namespace {

class UndoStackTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: UndoStackTest()
      : buffer_(new text::Buffer(L"*test*", new text::PlainTextMode())) {
  }
  public: virtual ~UndoStackTest() {
  }

  public: text::Buffer* buffer() const { return buffer_.get(); }

  DISALLOW_COPY_AND_ASSIGN(UndoStackTest);
};

TEST_F(UndoStackTest, Delete) {
  buffer()->Insert(0, L"foo");
  buffer()->Delete(0, 3);
  buffer()->Undo(0);
  EXPECT_EQ(3, buffer()->GetEnd());
  buffer()->Redo(3);
  EXPECT_EQ(0, buffer()->GetEnd());
}

TEST_F(UndoStackTest, Group) {
  {
    text::UndoBlock undo_group(buffer(), L"test");
    buffer()->Insert(0, L"foo");
    buffer()->Insert(1, L"bar");
  }
  buffer()->Undo(4);
  EXPECT_EQ(0, buffer()->GetEnd());
}

TEST_F(UndoStackTest, Insert) {
  buffer()->Insert(0, L"foo");
  EXPECT_EQ(3, buffer()->Undo(2)) << "Move to the last editing position";
  EXPECT_EQ(3, buffer()->GetEnd()) <<
      "Undo isn't occured position other than last editing position.";
  EXPECT_EQ(0, buffer()->Undo(3)) << "Undo at the last editing position";
  EXPECT_EQ(0, buffer()->GetEnd());
  buffer()->Redo(0);
  EXPECT_EQ(3, buffer()->GetEnd());
}

TEST_F(UndoStackTest, Merge) {
  {
    text::UndoBlock undo_group(buffer(), L"test");
    buffer()->Insert(0, L"foo");
  }
  {
    text::UndoBlock undo_group(buffer(), L"test");
    buffer()->Insert(3, L"bar");
  }
  EXPECT_EQ(0, buffer()->Undo(6));
  EXPECT_EQ(0, buffer()->GetEnd());
}

}  // namespace