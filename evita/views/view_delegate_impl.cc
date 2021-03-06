// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/view_delegate_impl.h"

#include <sstream>

#include "base/logging.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/windows/text_window.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/editor/modal_message_loop_scope.h"
#include "evita/editor/switch_set.h"
#include "evita/gc/collector.h"
#include "evita/metrics/counter.h"
#include "evita/metrics/time_scope.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/text_field_control.h"
#include "evita/views/forms/file_dialog_box.h"
#include "evita/views/forms/form_control_set.h"
#include "evita/views/forms/form_window.h"
#include "evita/views/frame_list.h"
#include "evita/views/table_window.h"
#include "evita/views/tabs/tab_data_set.h"
#include "evita/views/text_window.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"

namespace views {

namespace {

Window* FromWindowId(const char* name, dom::WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << name << ": No such window " << window_id;
    return nullptr;
  }
  return window;
}

Frame* GetFrameForMessage(dom::WindowId window_id) {
  if (window_id == dom::kInvalidWindowId)
    return FrameList::instance()->active_frame();
  auto const window = FromWindowId("GetFrameForMessage", window_id);
  if (!window)
    return FrameList::instance()->active_frame();
  for (auto runner = static_cast<ui::Widget*>(window); runner;
       runner = runner->parent_node()) {
    if (auto const frame = runner->as<Frame>())
      return frame;
  }
  return FrameList::instance()->active_frame();
}

}  // namespace

ViewDelegateImpl::ViewDelegateImpl() : event_handler_(nullptr) {}

ViewDelegateImpl::~ViewDelegateImpl() {
  // event_handler_ should be nullptr
}

void ViewDelegateImpl::AddWindow(dom::WindowId parent_id,
                                 dom::WindowId child_id) {
  DCHECK_NE(dom::kInvalidWindowId, parent_id);
  auto const parent = Window::FromWindowId(parent_id);
  if (!parent) {
    DVLOG(0) << "AddWindow: no such parent " << parent_id;
    return;
  }
  DCHECK_EQ(parent_id, parent->window_id());
  auto const child = Window::FromWindowId(child_id);
  if (!child) {
    DVLOG(0) << "AddWindow: no such child " << child_id;
    return;
  }
  DCHECK_EQ(child_id, child->window_id());
  parent->as<Frame>()->AddOrActivateTabContent(
      child->as<views::ContentWindow>());
}

void ViewDelegateImpl::ChangeParentWindow(dom::WindowId window_id,
                                          dom::WindowId new_parent_id) {
  auto const window = FromWindowId("ChangeParentWindow", window_id);
  if (!window)
    return;
  auto const new_parent = FromWindowId("ChangeParentWindow", new_parent_id);
  if (!new_parent)
    return;
  window->SetParentWidget(new_parent);
}

text::Posn ViewDelegateImpl::ComputeOnTextWindow(
    dom::WindowId window_id,
    const dom::TextWindowCompute& data) {
  auto const window =
      FromWindowId("ComputeOnTextWindow", window_id)->as<TextWindow>();
  if (!window)
    return -1;
  gfx::PointF point(data.x, data.y);
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  switch (data.method) {
    case dom::TextWindowCompute::Method::EndOfWindow:
      return window->GetEnd();
    case dom::TextWindowCompute::Method::EndOfWindowLine:
      return window->EndOfLine(data.position);
    case dom::TextWindowCompute::Method::MoveScreen:
      return window->ComputeMotion(Unit_Screen, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::MoveWindow:
      return window->ComputeMotion(Unit_Window, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::MoveWindowLine:
      return window->ComputeMotion(Unit_WindowLine, data.count, point,
                                   data.position);
    case dom::TextWindowCompute::Method::StartOfWindow:
      return window->GetStart();
    case dom::TextWindowCompute::Method::StartOfWindowLine:
      return window->StartOfLine(data.position);
    default:
      return -1;
  }
}

void ViewDelegateImpl::CreateEditorWindow(const dom::EditorWindow* window) {
  DCHECK(window);
  new Frame(window->window_id());
}

void ViewDelegateImpl::CreateFormWindow(dom::WindowId window_id,
                                        dom::Form* form,
                                        const domapi::PopupWindowInit& init) {
  if (init.owner_id == dom::kInvalidWindowId) {
    new FormWindow(window_id, form);
    return;
  }
  auto const owner = Window::FromWindowId(init.owner_id);
  if (!owner) {
    DVLOG(0) << "CreateFormWindow: no such window " << init.owner_id;
    return;
  }
  new FormWindow(window_id, form, owner,
                 gfx::Point(init.offset_x, init.offset_y));
}

void ViewDelegateImpl::CreateTableWindow(dom::WindowId window_id,
                                         dom::Document* document) {
  new views::TableWindow(window_id, document);
}

void ViewDelegateImpl::CreateTextWindow(dom::WindowId window_id,
                                        text::Selection* selection) {
  new TextWindow(window_id, selection);
}

void ViewDelegateImpl::DestroyWindow(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "DestroyWindow: no such widget " << window_id;
    return;
  }
  widget->DidDestroyDomWindow();
  widget->DestroyWidget();
}

void ViewDelegateImpl::DidStartScriptHost(domapi::ScriptHostState state) {
  editor::Application::instance()->DidStartScriptHost(state);
}

void ViewDelegateImpl::FocusWindow(dom::WindowId window_id) {
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "FocusWindow: no such widget " << window_id;
    return;
  }
  widget->RequestFocus();
}

void ViewDelegateImpl::GetFileNameForLoad(
    dom::WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForLoadResolver& resolver) {
  auto const widget = window_id == dom::kInvalidWindowId
                          ? FrameList::instance()->active_frame()
                          : Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFileNameForLoad: no such widget " << window_id;
    event_handler_->RunCallback(base::Bind(resolver.resolve, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetOpenFileName(&params))
    return;
  event_handler_->RunCallback(
      base::Bind(resolver.resolve, base::string16(params.m_wsz)));
}

void ViewDelegateImpl::GetFileNameForSave(
    dom::WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForSaveResolver& resolver) {
  auto const widget = window_id == dom::kInvalidWindowId
                          ? FrameList::instance()->active_frame()
                          : Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "GetFileNameForSave: no such widget " << window_id;
    event_handler_->RunCallback(base::Bind(resolver.resolve, base::string16()));
    return;
  }
  FileDialogBox::Param params;
  params.SetDirectory(dir_path.c_str());
  params.m_hwndOwner = widget->AssociatedHwnd();
  FileDialogBox oDialog;
  if (!oDialog.GetSaveFileName(&params))
    return;
  event_handler_->RunCallback(
      base::Bind(resolver.resolve, base::string16(params.m_wsz)));
}

base::string16 ViewDelegateImpl::GetMetrics(const base::string16& name) {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());

  base::string16 delimiter = L"";
  const base::string16 comma = L",\n";

  std::basic_ostringstream<base::char16> ostream;
  ostream << '{';

  auto const times = metrics::HistogramSet::instance()->GetJson(name);
  if (!times.empty()) {
    ostream << delimiter << L"\"times\": " << times;
    delimiter = comma;
  }

  auto const counters = metrics::CounterSet::instance()->GetJson(name);
  if (!counters.empty()) {
    ostream << delimiter << L"\"counters\": " << counters;
    delimiter = comma;
  }

  auto const objects = gc::Collector::instance()->GetJson(name);
  if (!objects.empty()) {
    ostream << delimiter << L"\"objects\": " << objects;
    delimiter = comma;
  }

  ostream << '}';
  return ostream.str();
}

domapi::SwitchValue ViewDelegateImpl::GetSwitch(const base::string16& name) {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return editor::SwitchSet::instance()->Get(name);
}

std::vector<base::string16> ViewDelegateImpl::GetSwitchNames() {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return editor::SwitchSet::instance()->names();
}

std::vector<int> ViewDelegateImpl::GetTableRowStates(
    WindowId window_id,
    const std::vector<base::string16>& keys) {
  auto const widget = FromWindowId("GetTableRowStates", window_id);
  if (!widget)
    return std::vector<int>();
  auto const table_view = widget->as<views::TableWindow>();
  if (!table_view)
    return std::vector<int>();
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  return std::move(table_view->GetRowStates(keys));
}

void ViewDelegateImpl::HideWindow(dom::WindowId window_id) {
  auto const window = FromWindowId("HideWindow", window_id);
  if (!window)
    return;
  window->Hide();
}

domapi::FloatRect ViewDelegateImpl::HitTestTextPosition(WindowId window_id,
                                                        text::Posn position) {
  auto const window =
      FromWindowId("HitTestTextPosition", window_id)->as<TextWindow>();
  if (!window)
    return domapi::FloatRect();
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  const auto rect = window->HitTestTextPosition(position);
  return domapi::FloatRect(rect.left, rect.top, rect.width(), rect.height());
}

void ViewDelegateImpl::MakeSelectionVisible(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget) {
    DVLOG(0) << "MakeSelectionVisible: no such widget " << window_id;
    return;
  }
  auto const content_window = widget->as<ContentWindow>();
  if (!content_window) {
    DVLOG(0) << "MakeSelectionVisible: not ContentWindow" << window_id;
    return;
  }
  UI_DOM_AUTO_LOCK_SCOPE();
  content_window->MakeSelectionVisible();
}

text::Posn ViewDelegateImpl::MapPointToPosition(
    domapi::EventTargetId event_target_id,
    float x,
    float y) {
  if (auto const window = Window::FromWindowId(event_target_id)) {
    if (auto const text_window = window->as<TextWindow>()) {
      UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
      DCHECK(lock_scope.locked());
      return text_window->MapPointToPosition(gfx::PointF(x, y));
    }
  }

  if (auto const control =
          FormControlSet::instance()->MaybeControl(event_target_id)) {
    if (auto const text_field = control->as<ui::TextFieldControl>())
      return text_field->MapPointToOffset(gfx::PointF(x, y));
  }

  return 0;
}

void ViewDelegateImpl::MessageBox(dom::WindowId window_id,
                                  const base::string16& message,
                                  const base::string16& title,
                                  int flags,
                                  const MessageBoxResolver& resolver) {
  auto const frame = GetFrameForMessage(window_id);

  auto const kButtonMask = 7;
  auto const kIconMask = 0x70;
  auto const need_response = flags & kButtonMask;
  auto const level = (flags & kIconMask) == MB_ICONERROR
                         ? MessageLevel_Error
                         : (flags & kIconMask) == MB_ICONWARNING
                               ? MessageLevel_Warning
                               : MessageLevel_Information;

  if (!need_response && level != MessageLevel_Error) {
    if (frame)
      frame->ShowMessage(level, message);
    event_handler_->RunCallback(base::Bind(resolver.resolve, IDOK));
    return;
  }

  auto safe_title = title;
  if (!safe_title.empty())
    safe_title += L" - ";
  safe_title += editor::Application::instance()->title();
  auto const hwnd = frame ? frame->AssociatedHwnd() : nullptr;
  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  auto const response = ::MessageBoxW(hwnd, message.c_str(), title.c_str(),
                                      static_cast<UINT>(flags));
  event_handler_->RunCallback(base::Bind(resolver.resolve, response));
}

void ViewDelegateImpl::Reconvert(WindowId window_id,
                                 const base::string16& text) {
  auto const window = FromWindowId("Reconvert", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextWindow>();
  if (!text_window) {
    DVLOG(0) << "WindowId " << window_id << " should be TextWindow.";
    return;
  }
  ui::TextInputClient::Get()->Reconvert(text_window, text);
}

void ViewDelegateImpl::RealizeWindow(dom::WindowId window_id) {
  DCHECK_NE(dom::kInvalidWindowId, window_id);
  auto const widget = Window::FromWindowId(window_id);
  if (!widget)
    return;
  DCHECK_EQ(window_id, widget->window_id());
  widget->RealizeWidget();
}

void ViewDelegateImpl::RegisterViewEventHandler(
    domapi::ViewEventHandler* event_handler) {
  DCHECK(!event_handler_);
  event_handler_ = event_handler;
  event_handler_->DidStartViewHost();
}

void ViewDelegateImpl::SetTextWindowZoom(dom::WindowId window_id, float zoom) {
  DCHECK_GT(zoom, 0.0f);
  auto const window = FromWindowId("SetTextWindowZoom", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextWindow>();
  if (!text_window)
    return;
  text_window->SetZoom(zoom);
}

void ViewDelegateImpl::ShowWindow(dom::WindowId window_id) {
  auto const window = FromWindowId("ShowWindow", window_id);
  if (!window)
    return;
  window->Show();
}

void ViewDelegateImpl::ReleaseCapture(domapi::EventTargetId event_target_id) {
  if (auto const window = Window::FromWindowId(event_target_id)) {
    window->ReleaseCapture();
    return;
  }

  if (auto const control =
          FormControlSet::instance()->MaybeControl(event_target_id)) {
    control->ReleaseCapture();
    return;
  }

  DVLOG(0) << "ReleaseCapture: no such target " << event_target_id;
}

void ViewDelegateImpl::ScrollTextWindow(WindowId window_id, int direction) {
  auto const window = FromWindowId("ScrollTextWindow", window_id);
  if (!window)
    return;
  auto const text_window = window->as<TextWindow>();
  if (!text_window) {
    DVLOG(0) << "ScrollTextWindow expects TextWindow.";
    return;
  }
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  text_window->SmallScroll(0, direction);
}

void ViewDelegateImpl::SetCapture(domapi::EventTargetId event_target_id) {
  if (auto const window = Window::FromWindowId(event_target_id)) {
    window->SetCapture();
    return;
  }

  if (auto const control =
          FormControlSet::instance()->MaybeControl(event_target_id)) {
    control->SetCapture();
    return;
  }

  DVLOG(0) << "SetCapture: no such target " << event_target_id;
}

void ViewDelegateImpl::SetStatusBar(dom::WindowId window_id,
                                    const std::vector<base::string16>& texts) {
  auto const window = FromWindowId("SetStatusBar", window_id);
  if (!window)
    return;
  auto const frame = window->as<Frame>();
  if (!frame) {
    DVLOG(0) << "Window " << window_id << " should be Frame window.";
    return;
  }
  frame->SetStatusBar(texts);
}

void ViewDelegateImpl::SetSwitch(const base::string16& name,
                                 const domapi::SwitchValue& new_value) {
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  DCHECK(lock_scope.locked());
  editor::SwitchSet::instance()->Set(name, new_value);
}

void ViewDelegateImpl::SetTabData(dom::WindowId window_id,
                                  const domapi::TabData& tab_data) {
  TabDataSet::instance()->SetTabData(window_id, tab_data);
}

void ViewDelegateImpl::SplitHorizontally(dom::WindowId left_window_id,
                                         dom::WindowId new_right_window_id) {
  auto const left_window = Window::FromWindowId(left_window_id);
  if (!left_window)
    return;
  auto const new_right_window = Window::FromWindowId(new_right_window_id);
  if (!new_right_window)
    return;
  auto const parent = left_window->parent_node()->as<EditPane>();
  if (!parent)
    return;
  parent->SplitHorizontally(left_window->as<ContentWindow>(),
                            new_right_window->as<ContentWindow>());
}

void ViewDelegateImpl::SplitVertically(dom::WindowId above_window_id,
                                       dom::WindowId new_below_window_id) {
  auto const above_window = Window::FromWindowId(above_window_id);
  if (!above_window)
    return;
  auto const new_below_window = Window::FromWindowId(new_below_window_id);
  if (!new_below_window)
    return;
  auto const parent = above_window->parent_node()->as<EditPane>();
  if (!parent)
    return;
  parent->SplitVertically(above_window->as<ContentWindow>(),
                          new_below_window->as<ContentWindow>());
}

void ViewDelegateImpl::UpdateWindow(dom::WindowId window_id) {
  auto const window = FromWindowId("UpdateWindow", window_id);
  if (!window)
    return;
  auto const now = base::Time::Now();
  window->HandleAnimationFrame(now);
}

}  // namespace views
