// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor.h"

#include "evita/cm_CmdProc.h"
#include "evita/gc/local.h"
#include "evita/dom/editor_window.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_command.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/application.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {

void AddKeyBinding(int key_code, v8::Handle<v8::Object> command) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  ASSERT_DOM_LOCKED();
  Command::g_pGlobalBinds->Bind(key_code, new ScriptCommand(command));
}

// TODO(yosi): Until we enable |new EditorWindow()|, we use
// |editor.createFrame()|.
v8::Handle<v8::Object> CreateFrame() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  ASSERT_DOM_LOCKED();
  auto window = gc::MakeLocal(new EditorWindow());
  return window->GetWrapper(v8::Isolate::GetCurrent());
}

}  // namespace

Editor::Editor() {
}

v8_glue::WrapperInfo* Editor::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info,
      ("Editor", "editor"));
  return &wrapper_info;
}

const base::string16& Editor::version() const {
  return Application::instance()->version();
}

gin::ObjectTemplateBuilder Editor::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetObjectTemplateBuilderFromBase(isolate)
      .SetMethod("addKeyBinding", AddKeyBinding)
      .SetMethod("createFrame", CreateFrame)
      .SetProperty("version", &Editor::version);
}

}  // namespace dom
