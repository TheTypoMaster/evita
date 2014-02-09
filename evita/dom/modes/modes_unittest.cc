// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class ModesTest : public dom::AbstractDomTest {
  public: ModesTest() = default;
  public: ~ModesTest() = default;

  DISALLOW_COPY_AND_ASSIGN(ModesTest);
};

TEST_F(ModesTest, ConfigMode) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var mode = new ConfigMode(doc);");
  EXPECT_SCRIPT_TRUE("mode instanceof Mode");
  EXPECT_SCRIPT_TRUE("mode instanceof ConfigMode");
  EXPECT_SCRIPT_TRUE("mode.document === doc");
  EXPECT_SCRIPT_EQ("Config", "mode.name");
}

}  // namespace
