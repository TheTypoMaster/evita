// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/run_loop.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::Eq;
using ::testing::_;

class TextWindowTest : public dom::AbstractDomTest {
  protected: TextWindowTest() {
  }
  public: virtual ~TextWindowTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextWindowTest);
};

TEST_F(TextWindowTest, _ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "var sample = new TextWindow(range);");
  EXPECT_SCRIPT_TRUE("sample instanceof TextWindow");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_EQ("1", "sample.id");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_TRUE("sample.selection instanceof Selection");
}

TEST_F(TextWindowTest, clone) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_)).Times(2);
  EXPECT_SCRIPT_VALID(
      "var original = new TextWindow(new Range(new Document('foo')));"
      "original.selection.range.text = 'foo';"
      "original.selection.range.end = 3;"
      "var sample = original.clone();");
  EXPECT_SCRIPT_EQ("3", "sample.selection.range.end");
}

TEST_F(TextWindowTest, makeSelectionVisible) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_CALL(*mock_view_impl(), MakeSelectionVisible(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var sample = new TextWindow(new Range(new Document('foo')));"
      "sample.makeSelectionVisible();");
}

TEST_F(TextWindowTest, realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var sample = new TextWindow(new Range(doc));"
      "var event;"
      "function event_handler(x) { event = x; }"
      "doc.addEventListener('attach', event_handler);"
      "doc.addEventListener('detach', event_handler);");

  base::MessageLoop message_loop;

  // The document receives "attach" event when |TextWindow| is realized.
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.realize()");
  view_event_handler()->DidRealizeWidget(1);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_SCRIPT_TRUE("event instanceof DocumentEvent");
  EXPECT_SCRIPT_EQ("attach", "event.type");
  EXPECT_SCRIPT_TRUE("event.view === sample");

  // The document receives "detach" event when |TextWindow| is destroyed.
  EXPECT_CALL(*mock_view_impl(), DestroyWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.destroy()");
  view_event_handler()->DidDestroyWidget(1);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_SCRIPT_TRUE("event instanceof DocumentEvent");
  EXPECT_SCRIPT_EQ("detach", "event.type");
  EXPECT_SCRIPT_TRUE("event.view === sample");
}

}  // namespace