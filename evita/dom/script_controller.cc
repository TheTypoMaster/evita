// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/script_controller.h"

#include "base/logging.h"
#include "evita/dom/console.h"
#include "evita/dom/editor.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/script_thread.h"
#include "evita/dom/window.h"
#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

namespace dom {

namespace {

template<typename T>
class Installer {
  public: static void Run(v8::Isolate* isolate,
                          v8::Handle<v8::ObjectTemplate> global) {
    InstallConstructor(global, T::StaticGetConstructor(isolate));
    auto const info = T::static_wrapper_info();
    DCHECK(!info->singleton_name());
  }
  public: static void RunSingleton(v8::Isolate* isolate,
                                   v8::Handle<v8::ObjectTemplate> global) {
    InstallConstructor(global, T::StaticGetConstructor(isolate));
    auto const info = T::static_wrapper_info();
    DCHECK(info->singleton_name());
    global->Set(gin::StringToV8(isolate, info->singleton_name()),
                T::instance()->GetWrapper(isolate));
  }

  private: static void InstallConstructor(
      v8::Handle<v8::ObjectTemplate> templ,
      v8::Handle<v8::Function> constructor) {
    templ->Set(constructor->GetName()->ToString(), constructor);
  }
};

class Initializer {
  public: static v8::Handle<v8::Context> CreateContext(v8::Isolate* isolate) {
    return v8::Context::New(isolate, nullptr, GetGlobalTemplate(isolate));
  }

  private: static v8::Handle<v8::ObjectTemplate> GetGlobalTemplate(
      v8::Isolate* isolate) {
    DEFINE_STATIC_LOCAL(v8::Persistent<v8::ObjectTemplate>, global_template);
    if (!global_template.IsEmpty())
      return v8::Local<v8::ObjectTemplate>::New(isolate, global_template);

    auto global = v8::ObjectTemplate::New(isolate);
    {
        auto context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);

        // Note: super class must be installed before subclass.
        Installer<Console>::RunSingleton(isolate, global);
        Installer<Editor>::RunSingleton(isolate, global);
        Installer<Window>::Run(isolate, global);
        Installer<EditorWindow>::Run(isolate, global);
    }

    global_template.Reset(isolate, global);
    return v8::Local<v8::ObjectTemplate>::New(isolate, global_template);
  }
};

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        string_value.length());
}

void ReportException(base::Callback<void(EvaluateResult)> callback,
                     const v8::TryCatch& try_catch) {
  EvaluateResult eval_result;
  auto message = try_catch.Message();
  eval_result.exception = V8ToString(try_catch.Exception());
  if (!message.IsEmpty()) {
    eval_result.script_resource_name = V8ToString(
        message->GetScriptResourceName());
    eval_result.line_number = message->GetLineNumber();
    eval_result.source_line = V8ToString(message->GetSourceLine());
    eval_result.start_column = message->GetStartColumn();
    eval_result.end_column = message->GetEndColumn();
  }

  auto stack_trace = try_catch.StackTrace();
  if (!stack_trace.IsEmpty() && stack_trace->IsArray()) {
    auto array = stack_trace.As<v8::Array>();
    eval_result.stack_trace.resize(array->Length());
    auto index = 0u;
    for (auto dst: eval_result.stack_trace) {
      auto entry = array->CloneElementAt(index);
      dst = V8ToString(entry);
      ++index;
    }
  }

  Application::instance()->PostDomTask(FROM_HERE,
      base::Bind(callback, eval_result));
}

// TODO(yosi) Once we make ScriptThread at start up, we don't need to have
// |HackEvaluate|.
void HackEvaluate(base::string16 script_text,
                  base::Callback<void(EvaluateResult)> callback) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  ScriptController::instance()->Evaluate(script_text, callback);
}

}   // namespace

ScriptController::ScriptController()
    : context_holder_(isolate_holder_.isolate()) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  isolate_holder_.isolate()->Enter();
  {
    v8::HandleScope handle_scope(isolate_holder_.isolate());
    auto context = Initializer::CreateContext(isolate_holder_.isolate());
    context_holder_.SetContext(context);
    context->Enter();
  }
}

ScriptController::~ScriptController() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  {
    v8::HandleScope handle_scope(isolate_holder_.isolate());
    context_holder_.context()->Exit();
  }
  isolate_holder_.isolate()->Exit();
}

void ScriptController::Evaluate(
    base::string16 script_text,
    base::Callback<void(EvaluateResult)> callback) {

  DOM_AUTO_LOCK_SCOPE();

  DCHECK(thread_checker_.CalledOnValidThread());
  v8::HandleScope handle_scope(isolate_holder_.isolate());
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(
      v8::String::NewFromTwoByte(isolate_holder_.isolate(),
                                 reinterpret_cast<uint16*>(&script_text[0]),
                                 v8::String::kNormalString,
                                 script_text.length()),
      v8::String::NewFromUtf8(isolate_holder_.isolate(), "(console)"));
  if (script.IsEmpty()) {
    ReportException(callback, try_catch);
    return;
  }
  v8::Handle<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    DCHECK(try_catch.HasCaught());
    ReportException(callback, try_catch);
    return;
  }

  EvaluateResult eval_result;
  eval_result.value = V8ToString(result->ToString());
  Application::instance()->PostDomTask(FROM_HERE,
      base::Bind(callback, eval_result));
}

//////////////////////////////////////////////////////////////////////
//
// ScriptController::User
//
void ScriptController::User::Evaluate(
    base::string16 script_text,
    base::Callback<void(EvaluateResult)> callback) {
  ASSERT_CALLED_ON_UI_THREAD();
  ScriptThread::instance()->PostTask(
      FROM_HERE,
      base::Bind(HackEvaluate, script_text, callback));
}

}  // namespace dom
