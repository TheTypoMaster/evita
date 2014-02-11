// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os_file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/object_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "v8_strings.h"

namespace gin {
template<>
struct Converter<base::Time> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate, base::Time file_time);
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     base::Time* out);
};

v8::Handle<v8::Value> Converter<base::Time>::ToV8(v8::Isolate* isolate,
                                                  base::Time time) {
  return v8::Date::New(isolate, time.ToJsTime());
}

bool Converter<base::Time>::FromV8(v8::Isolate*, v8::Handle<v8::Value> val,
                                   base::Time* out) {
  if (!val->IsDate())
    return false;
  auto const date = v8::Handle<v8::Date>::Cast(val);
  *out = base::Time::FromJsTime(date->ValueOf());
  return true;
}
}  // namespace gin

namespace dom {

namespace os {
namespace file {
namespace {

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusCallback
//
class QueryFileStatusCallback
    : public base::RefCounted<QueryFileStatusCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: QueryFileStatusCallback(v8_glue::Runner* runner,
                                  v8::Handle<v8::Function> function)
      : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: ~QueryFileStatusCallback() = default;

  public: void Run(const domapi::QueryFileStatusCallbackData& data) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    auto const js_data = v8::Object::New(isolate);
    js_data->Set(v8Strings::errorCode.Get(isolate),
                 gin::ConvertToV8(isolate, data.error_code));
    js_data->Set(v8Strings::isDir.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_directory));
    js_data->Set(v8Strings::isSymLink.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_symlink));
    js_data->Set(v8Strings::lastModificationDate.Get(isolate),
                 gin::ConvertToV8(isolate, data.last_write_time));
    js_data->Set(v8Strings::readonly.Get(isolate),
                 gin::ConvertToV8(isolate, data.readonly));
    runner_->Call(function, v8::Undefined(isolate), js_data);
    js_data->Set(v8Strings::size.Get(isolate),
                 gin::ConvertToV8(isolate, data.file_size));
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusCallback);
};

void QueryFileStatus(const base::string16& filename,
                     v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const query_file_status_callback = make_scoped_refptr(
      new QueryFileStatusCallback(runner, callback));
  ScriptController::instance()->io_delegate()->QueryFileStatus(
      filename, base::Bind(&QueryFileStatusCallback::Run,
                           query_file_status_callback));
}

}  // namespace

v8::Handle<v8::ObjectTemplate> CreateObjectTemplate(v8::Isolate* isolate) {
  gin::ObjectTemplateBuilder builder(isolate);
  return builder.SetMethod("stat_", &QueryFileStatus).Build();
}

}  // namespace file
}  // namespace os
}  // namespace dom
