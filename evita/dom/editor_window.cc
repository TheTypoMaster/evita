// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/editor_window.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "common/memory/singleton.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"


namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// EditorWindowList
//
class EditorWindowList : public common::Singleton<EditorWindowList> {
  friend class common::Singleton<EditorWindowList>;

  private: typedef std::vector<EditorWindow*> List;
  private: std::unordered_set<EditorWindow*> set_;

  private: EditorWindowList() = default;
  public: ~EditorWindowList() = default;

  private: List list() const {
    List list(set_.size());
    list.resize(0);
    for (auto window : set_) {
      list.push_back(window);
    }
    return std::move(list);
  }

  public: void Register(EditorWindow* window) {
    set_.insert(window);
  }
  public: void ResetForTesting() {
    set_.clear();
  }
  public: static List StaticList() {
    return instance()->list();
  }
  public: void Unregister(EditorWindow* window) {
    set_.erase(window);
  }
};

//////////////////////////////////////////////////////////////////////
//
// EditorWindowWrapperInfo
//
class EditorWindowWrapperInfo : public v8_glue::WrapperInfo {
  public: EditorWindowWrapperInfo() : v8_glue::WrapperInfo("EditorWindow") {
  }
  public: ~EditorWindowWrapperInfo() = default;

  private: virtual WrapperInfo* inherit_from() const override {
    return Window::static_wrapper_info();
  }

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &EditorWindowWrapperInfo::NewEditorWindow);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetProperty("list", &EditorWindowList::StaticList)
        .Build();
  }

  private: static EditorWindow* NewEditorWindow() {
    return new EditorWindow();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    v8_glue::WrapperInfo::SetupInstanceTemplate(builder);
    // TODO(yosi) Add EditorWindow properties.
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EditorWindow
//
EditorWindow::EditorWindow() {
  EditorWindowList::instance()->Register(this);
  ScriptController::instance()->view_delegate()->CreateEditorWindow(this);
}

EditorWindow::~EditorWindow() {
  EditorWindowList::instance()->Unregister(this);
}

v8_glue::WrapperInfo* EditorWindow::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EditorWindowWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

void EditorWindow::ResetForTesting() {
  EditorWindowList::instance()->ResetForTesting();
}

}  // namespace dom
