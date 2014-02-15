// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class ButtonControlTest : public dom::AbstractDomTest {
  protected: ButtonControlTest() {
  }
  public: virtual ~ButtonControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(ButtonControlTest);
};

TEST_F(ButtonControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new ButtonControl(123);");
  EXPECT_SCRIPT_EQ("123", "sample.controlId");
}

TEST_F(ButtonControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new ButtonControl(123);"
      "var clicked = false;"
      "sample.addEventListener('click', function() { clicked = true; });"
      "sample.dispatchEvent(new FormEvent('click'));");
  EXPECT_SCRIPT_EQ("true", "clicked");
}

}  // namespace