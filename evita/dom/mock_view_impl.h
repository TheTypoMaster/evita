// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mock_view_impl_h)
#define INCLUDE_evita_dom_mock_view_impl_h

#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4365 4628)
#include "gmock/gmock.h"
#pragma warning(pop)
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/view_delegate.h"

namespace dom {

class MockViewImpl final : public dom::ViewDelegate {
  private: bool check_spelling_result_;
  private: std::vector<base::string16> spelling_suggestions_;

  public: MockViewImpl();
  public: ~MockViewImpl() final;

  // ViewDelegate
  MOCK_METHOD2(AddWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ChangeParentWindow, void(WindowId, WindowId));
  MOCK_METHOD2(ComputeOnTextWindow,
      text::Posn(WindowId, const TextWindowCompute&));
  MOCK_METHOD1(CreateEditorWindow, void(const EditorWindow*));
  MOCK_METHOD3(CreateFormWindow, void(WindowId, Form*,
                                      const domapi::PopupWindowInit&));
  MOCK_METHOD2(CreateTableWindow, void(WindowId, Document*));
  MOCK_METHOD2(CreateTextWindow, void(WindowId, text::Selection*));
  MOCK_METHOD1(DestroyWindow, void(WindowId));
  MOCK_METHOD1(DidStartScriptHost, void(ScriptHostState));
  MOCK_METHOD1(FocusWindow, void(WindowId));
  public: void GetFileNameForLoad(WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForLoadResolver& resolver) final;
  public: void GetFileNameForSave(WindowId window_id,
    const base::string16& dir_path,
    const GetFileNameForSaveResolver& resolver) final;
  MOCK_METHOD1(GetMetrics, base::string16(const base::string16&));
  MOCK_METHOD1(GetSwitch, domapi::SwitchValue(const base::string16& name));
  MOCK_METHOD0(GetSwitchNames, std::vector<base::string16>());
  public: std::vector<int> GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys) final;
  MOCK_METHOD1(HideWindow, void(WindowId));
  MOCK_METHOD2(HitTestTextPosition,
               domapi::FloatRect(WindowId, text::Posn));
  MOCK_METHOD1(MakeSelectionVisible, void(WindowId));
  MOCK_METHOD3(MapPointToPosition,
      text::Posn(domapi::EventTargetId, float x, float y));
  public: void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      const MessageBoxResolver& resolver) final;
  MOCK_METHOD2(Reconvert, void(WindowId, const base::string16&));
  MOCK_METHOD1(RealizeWindow, void(WindowId));
  MOCK_METHOD1(RegisterViewEventHandler, void(domapi::ViewEventHandler*));
  MOCK_METHOD1(ReleaseCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(ScrollTextWindow, void(WindowId, int));
  MOCK_METHOD1(SetCapture, void(domapi::EventTargetId));
  MOCK_METHOD2(SetStatusBar,
      void(WindowId, const std::vector<base::string16>&));
  MOCK_METHOD2(SetSwitch, void(const base::string16&,
                               const domapi::SwitchValue&));
  MOCK_METHOD2(SetTabData, void(WindowId, const domapi::TabData&));
  MOCK_METHOD2(SetTextWindowZoom, void(WindowId, float));
  MOCK_METHOD1(ShowWindow, void(WindowId));
  MOCK_METHOD2(SplitHorizontally, void(WindowId, WindowId));
  MOCK_METHOD2(SplitVertically, void(WindowId, WindowId));
  MOCK_METHOD1(UpdateWindow, void(WindowId));

  DISALLOW_COPY_AND_ASSIGN(MockViewImpl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_view_impl_h)
