// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_V8_GLUE_ARRAY_BUFFER_VIEW_H_
#define EVITA_V8_GLUE_ARRAY_BUFFER_VIEW_H_

#include "evita/v8_glue/v8.h"
#include "gin/array_buffer.h"

namespace gin {
template <>
struct Converter<ArrayBufferView*> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val,
                     ArrayBufferView** out);
};
}  // namespace gin

#endif  // EVITA_V8_GLUE_ARRAY_BUFFER_VIEW_H_
