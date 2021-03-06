// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::_;

class DocumentEventTest : public dom::AbstractDomTest {
  protected: DocumentEventTest() {
  }
  public: virtual ~DocumentEventTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentEventTest);
};

TEST_F(DocumentEventTest, ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var window = new TextWindow(new Range(doc));"
      "var event = new DocumentEvent('foo', {"
      "  view: window,"
      "});");
  EXPECT_SCRIPT_FALSE("event.bubbles") << "default |bubbles| is false";
  EXPECT_SCRIPT_FALSE("event.cancelable");
  EXPECT_SCRIPT_TRUE("event.currentTarget == null");
  EXPECT_SCRIPT_FALSE("event.defaultPrevented");
  EXPECT_SCRIPT_TRUE("Event.PhaseType.NONE === event.eventPhase");
  EXPECT_SCRIPT_EQ("0", "event.timeStamp");
  EXPECT_SCRIPT_TRUE("event.target == null");
  EXPECT_SCRIPT_EQ("foo", "event.type");

  // DocumentEvent
  EXPECT_SCRIPT_TRUE("event.view == window");
}

}  // namespace
