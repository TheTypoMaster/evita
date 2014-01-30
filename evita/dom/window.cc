// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

#include <algorithm>
#include <unordered_map>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "evita/dom/events/event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/window_ostream.h"
#include "evita/gc/weak_ptr.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"
#include "v8_strings.h"

namespace dom {
namespace internal {
const char* WindowStateString(dom::Window::State state) {
  static const char* const state_strings[] = {
    "destroyed",
    "destroying",
    "notrealized",
    "realizing",
    "realized",
  };
  auto const index = static_cast<size_t>(static_cast<int>(state) -
      static_cast<int>(dom::Window::State::Destroyed));
  DCHECK_LE(index, arraysize(state_strings) - 1);
  return state_strings[index];
}
}  // namespace internal
}  // namespace dom

namespace gin {

template<>
struct Converter<dom::Window::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Window::State state) {
    return gin::StringToSymbol(isolate,
                               dom::internal::WindowStateString(state));
  }
};

} // namespace gin

namespace dom {

namespace {
class WindowWrapperInfo :
    public v8_glue::DerivedWrapperInfo<Window, EventTarget> {

  public: explicit WindowWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~WindowWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &WindowWrapperInfo::NewWindow);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetValue("focus", v8::Maybe<Window*>(false, nullptr))
        .Build();
  }

  private: static Window* NewWindow() {
    ScriptController::instance()->ThrowError(
        "You can't create Window instance.");
    return nullptr;
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetValue("bottom", 0)
        .SetProperty("children", &Window::child_windows)
        .SetProperty("firstChild", &Window::first_child)
        .SetValue("focusTick_", 0)
        .SetProperty("id", &Window::id)
        .SetProperty("lastChild", &Window::last_child)
        .SetValue("left", 0)
        .SetProperty("nextSibling", &Window::next_sibling)
        .SetProperty("parent", &Window::parent_window)
        .SetProperty("previousSibling", &Window::previous_sibling)
        .SetValue("right", 0)
        .SetProperty("state", &Window::state)
        .SetValue("top", 0)
        .SetMethod("appendChild", &Window::AddWindow)
        .SetMethod("changeParent", &Window::ChangeParentWindow)
        .SetMethod("destroy", &Window::Destroy)
        .SetMethod("focus", &Window::Focus)
        .SetMethod("realize", &Window::Realize)
        .SetMethod("removeChild", &Window::RemoveWindow)
        .SetMethod("splitHorizontally", &Window::SplitHorizontally)
        .SetMethod("splitVertically", &Window::SplitVertically);
  }
};

int global_focus_tick;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window::WindowIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
// WindowIdMapper resets Window::window_id() when corresponding widget is
// destroyed.
class Window::WindowIdMapper : public common::Singleton<WindowIdMapper> {
  friend class common::Singleton<WindowIdMapper>;

  private: typedef WindowId WindowId;

  private: std::unordered_map<WindowId, gc::WeakPtr<Window>> map_;

  private: WindowIdMapper() = default;
  public: ~WindowIdMapper() = default;

  public: void DidDestroyWidget(WindowId window_id) {
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a widget for WindowId " << window_id <<
        " in WindowIdMap?";
      return;
    }
    auto const window = it->second.get();
    DCHECK_NE(State::Destroyed, window->state_);
    window->state_ = State::Destroyed;
    if (auto const parent = window->parent_node()) {
      parent->RemoveChild(window);
    }
  }

  public: Window* Find(WindowId window_id) {
    auto it = map_.find(window_id);
    return it == map_.end() ? nullptr : it->second.get();
  }

  public: void Register(Window* window) {
    map_[window->window_id()] = window;
  }

  public: void ResetForTesting() {
    map_.clear();
  }

  public: void Unregister(WindowId window_id) {
    DCHECK_NE(kInvalidWindowId, window_id);
    map_.erase(window_id);
  }
};

//////////////////////////////////////////////////////////////////////
//
// Window
//
DEFINE_SCRIPTABLE_OBJECT(Window, WindowWrapperInfo)

Window::Window()
    : state_(State::NotRealized) {
  WindowIdMapper::instance()->Register(this);
}

Window::~Window() {
  if (window_id() == kInvalidWindowId)
    return;
  WindowIdMapper::instance()->Unregister(window_id());
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

std::vector<Window*> Window::child_windows() const {
  std::vector<Window*> child_windows;
  for (auto child : child_nodes()) {
    child_windows.push_back(const_cast<Window*>(child));
  }
  return child_windows;
}

void Window::AddWindow(Window* window) {
  if (window == this) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Can't add window(%d) to itself.", id()));
    return;
  }
  if (window->parent_node()) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is already child of window(%d).",
            window->id(), window->parent_node()->id()));
    return;
  }
  if (IsDescendantOf(window)) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is parent or ancestor of window(%d).",
            window->id(), id()));
    return;
  }
  AppendChild(window);
  ScriptController::instance()->view_delegate()->AddWindow(
      window_id(), window->window_id());
}

void Window::ChangeParentWindow(Window* new_parent_window) {
  if (parent_node() == new_parent_window)
    return;
  if (this == new_parent_window) {
    ScriptController::instance()->ThrowError(
      "Can't change parent to itself.");
    return;
  }
  if (new_parent_window->IsDescendantOf(this)) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "Can't change parent of window(%d) to window(%d), becase window(%d)"
        " is descendant of window(%d).",
        window_id(), new_parent_window->window_id(),
        new_parent_window->window_id(), window_id()));
    return;
  }
  if (parent_node())
    parent_node()->RemoveChild(this);
  new_parent_window->AppendChild(this);
  ScriptController::instance()->view_delegate()->ChangeParentWindow(
      window_id(), new_parent_window->window_id());
}

void Window::Destroy() {
  if (state_ != State::Realized && state_ != State::Realizing) {
    ScriptController::instance()->ThrowError(
        "You can't destroy unrealized window.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_= State::Destroying;
  }
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

void Window::DidDestroyWidget(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  WindowIdMapper::instance()->DidDestroyWidget(window_id);
}

// Possible state transitions:
//  State::Realizing -> State::Realized 
//    |realize()| call.
//  State::NotRealized -> State::Realized
//    Adding |State::NotRealized| window to |State::Realized| window.
//  State::Destroying -> State::Realized
//    The window was |State::Realizing| then |destroy()|.
void Window::DidRealizeWidget(WindowId window_id) {
  auto const widget = FromWindowId(window_id);
  DCHECK(widget->state_ == State::Realizing ||
         widget->state_ == State::Destroying ||
         widget->state_ == State::NotRealized);
  widget->state_ = State::Realized;
  for (auto child : widget->child_nodes()) {
    if (child->state_ == State::NotRealized)
      child->state_ = State::Realized;
  }
}

void Window::DidResize(int left, int top, int right, int bottom) {
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto const context = ScriptController::instance()->context();
  v8::Context::Scope context_scope(context);
  auto const instance = GetWrapper(isolate);
  instance->Set(v8Strings::left.Get(isolate), v8::Integer::New(isolate, left));
  instance->Set(v8Strings::top.Get(isolate), v8::Integer::New(isolate, top));
  instance->Set(v8Strings::right.Get(isolate),
                v8::Integer::New(isolate, right));
  instance->Set(v8Strings::bottom.Get(isolate),
                v8::Integer::New(isolate, bottom));
}

void Window::DidSetFocus() {
  ++global_focus_tick;

  // Update |Window.focus| and |Window.prototype.focusTick_|
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto const context = ScriptController::instance()->context();
  v8::Context::Scope context_scope(context);
  auto const instance = GetWrapper(isolate);
  auto const klass = context->Global()->Get(v8Strings::Window.Get(isolate));
  instance->Set(v8Strings::focusTick_.Get(isolate),
                v8::Integer::New(isolate, global_focus_tick));
  klass->ToObject()->Set(v8Strings::focus.Get(isolate), instance);
}

void Window::Focus() {
  if (state_ != State::Realized && state_ != State::Realizing) {
    ScriptController::instance()->ThrowError(
        "You can't focus unrealized window.");
    return;
  }
  ScriptController::instance()->view_delegate()->FocusWindow(window_id());
}

Window* Window::FromWindowId(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  return WindowIdMapper::instance()->Find(window_id);
}

bool Window::IsDescendantOf(Window* other) const {
  for (auto descendant : common::tree::descendants(other)) {
    if (descendant == this)
      return true;
  }
  return false;
}

void Window::Realize() {
  if (state_ == State::Destroyed) {
    DCHECK_EQ(kInvalidWindowId, window_id());
    ScriptController::instance()->ThrowError(
        "Can't realize deatched window.");
    return;
  }
  if (state_ == State::Realized) {
    ScriptController::instance()->ThrowError(
        "This window is already realized.");
    return;
  }
  if (state_ == State::Realizing) {
    ScriptController::instance()->ThrowError(
        "This window is being realized.");
    return;
  }
  if (parent_node() && parent_node()->state_ == State::NotRealized) {
    ScriptController::instance()->ThrowError(
        "Parent window isn't realized.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_= State::Realizing;
  }
  ScriptController::instance()->view_delegate()->RealizeWindow(window_id());
}

void Window::RemoveWindow(Window* window) {
  if (window->parent_node() != window) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "Can't remove window(%d) which isn't child of window(%d).",
        window->id(), id()));
    return;
  }
  parent_node()->RemoveChild(this);
}

void Window::ResetForTesting() {
  EventTarget::ResetForTesting();
  global_focus_tick = 0;
  WindowIdMapper::instance()->ResetForTesting();
}

static bool CheckSplitParameter(Window* ref_window, Window* new_window) {
  if (!ref_window->parent_node()) {
    ScriptController::instance()->ThrowError(
        "Can't split top-level window.");
    return false;
  }

  if (ref_window == new_window) {
    ScriptController::instance()->ThrowError(
        "Can't split window with itself.");
    return false;
  }

  if (ref_window->state() != Window::State::Realized) {
    ScriptController::instance()->ThrowError(
        "Can't split unrealized window.");
    return false;
  }

  if (new_window->parent_node()) {
    ScriptController::instance()->ThrowError(
        "Can't split with child window.");
    return false;
  }

  if (new_window->state() != Window::State::NotRealized) {
    ScriptController::instance()->ThrowError(
        "Can't split with realized window.");
    return false;
  }

  return true;
}

void Window::SplitHorizontally(Window* new_right_window) {
  if (!CheckSplitParameter(this, new_right_window))
    return;
  parent_node()->InsertAfter(new_right_window, this);
  ScriptController::instance()->view_delegate()->SplitHorizontally(
    id(), new_right_window->id());
}

void Window::SplitVertically(Window* new_below_window) {
  if (!CheckSplitParameter(this, new_below_window))
    return;
  parent_node()->InsertAfter(new_below_window, this);
  ScriptController::instance()->view_delegate()->SplitVertically(
    id(), new_below_window->id());
}

}  // namespace dom
