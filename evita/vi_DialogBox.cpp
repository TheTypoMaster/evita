#include "precomp.h"
// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_DialogBox.h"

#include <unordered_map>
#include <utility>

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "evita/editor/application.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;
extern HWND g_hwndActiveDialog;

namespace {

//////////////////////////////////////////////////////////////////////
//
// DialogBoxIdMapper
//
// This class represents mapping from widget id to DOM Dialog object.
//
class DialogBoxIdMapper : public common::Singleton<DialogBoxIdMapper> {
  friend class common::Singleton<DialogBoxIdMapper>;

  private: typedef DialogBoxId DialogBoxId;

  private: std::unordered_map<DialogBoxId, DialogBox*> map_;

  private: DialogBoxIdMapper() = default;
  public: ~DialogBoxIdMapper() = default;

  public: void DidDestroyDomDialog(DialogBoxId dialog_box_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidDialogBoxId, dialog_box_id);
    auto it = map_.find(dialog_box_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a dialog for DialogBoxId " <<
          dialog_box_id << " in DialogBoxIdMap?";
      return;
    }
    map_.erase(it);
  }

  public: DialogBox* Find(DialogBoxId dialog_box_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidDialogBoxId, dialog_box_id);
    auto it = map_.find(dialog_box_id);
    return it == map_.end() ? nullptr : it->second;
  }

  public: DialogBoxId Register(DialogBox* dialog) {
    ASSERT_CALLED_ON_UI_THREAD();
    auto const dialog_box_id = dialog->dialog_box_id();
    DCHECK_NE(kInvalidDialogBoxId, dialog_box_id);
    DCHECK_EQ(0u, map_.count(dialog_box_id));
    map_[dialog_box_id] = dialog;
    return dialog_box_id;
  }

  public: void Unregister(DialogBoxId dialog_box_id) {
    ASSERT_CALLED_ON_UI_THREAD();
    DCHECK_NE(kInvalidDialogBoxId, dialog_box_id);
    map_[dialog_box_id] = nullptr;
  }
};


DialogBox* creating_dialog_box;
}

DialogBox::DialogBox(DialogBoxId dialog_box_id)
    : dialog_box_id_(dialog_box_id) {
  DialogBoxIdMapper::instance()->Register(this);
}

DialogBox::~DialogBox() {
  DialogBoxIdMapper::instance()->Unregister(dialog_box_id_);
}

INT_PTR CALLBACK DialogBox::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam) {
  auto dialog_box = reinterpret_cast<DialogBox*>(
      ::GetWindowLongPtr(hwnd, DWLP_USER));

  if (!dialog_box) {
    dialog_box = creating_dialog_box;
    dialog_box->hwnd_ = hwnd;
    ::SetWindowLongPtr(hwnd, DWLP_USER,
                       reinterpret_cast<LONG_PTR>(dialog_box));
  }

  switch (uMsg) {
    case WM_ACTIVATE:
      DEBUG_PRINTF("WM_ACTIVATE %p wParam=%d\n", dialog_box, wParam);
      if (WA_INACTIVE == wParam)
        g_hwndActiveDialog = nullptr;
      else
        g_hwndActiveDialog = dialog_box->hwnd_;
      break;

    case WM_COMMAND:
      dialog_box->onCommand(wParam, lParam);
      return 0;

    case WM_INITDIALOG:
      return dialog_box->onInitDialog();
  }

  return dialog_box->onMessage(uMsg, wParam, lParam);
}

DialogBox* DialogBox::FromDialogBoxId(DialogBoxId dialog_box_id) {
  return DialogBoxIdMapper::instance()->Find(dialog_box_id);
}


bool DialogBox::GetChecked(int item_id) const {
  return BST_CHECKED == ::SendMessage(GetDlgItem(item_id), BM_GETCHECK, 0, 0);
}

HWND DialogBox::GetDlgItem(int item_id) const { 
  return ::GetDlgItem(hwnd_, item_id);
}

base::string16 DialogBox::GetDlgItemText(int item_id) const {
  auto const hwnd = GetDlgItem(item_id);
  auto const length = ::GetWindowTextLength(hwnd);
  if (!length)
    return base::string16();
  // +1 for terminating zero.
  base::string16 text(static_cast<size_t>(length + 1), '?');
  auto const length2 = ::GetWindowTextW(hwnd, &text[0],
                                        static_cast<int>(text.length()));
  DCHECK_EQ(length, length2);
  text.resize(static_cast<size_t>(length2));
  return std::move(text);
}

void DialogBox::onOk() {
  ::EndDialog(hwnd_, IDOK);
}

void DialogBox::onCancel() {
  ::EndDialog(hwnd_, IDCANCEL);
}

bool DialogBox::onCommand(WPARAM, LPARAM) {
  return false;
}

INT_PTR DialogBox::onMessage(UINT, WPARAM, LPARAM) {
  return 0;
}

void DialogBox::Realize() {
  creating_dialog_box = this;
  hwnd_ = ::CreateDialogParam(
      g_hInstance,
      MAKEINTRESOURCE(GetTemplate()),
      nullptr,
      DialogProc,
      reinterpret_cast<LPARAM>(this));
}

int DialogBox::SetCheckBox(int item_id, bool checked) {
  return static_cast<int>(::SendMessage(GetDlgItem(item_id), BM_SETCHECK,
      static_cast<WPARAM>(checked ? BST_CHECKED : BST_UNCHECKED), 0));
}

void DialogBox::Show() {
  ::ShowWindow(*this, SW_SHOW);
  ::SetActiveWindow(*this);
}
