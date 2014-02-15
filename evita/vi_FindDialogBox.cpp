#include "precomp.h"
// Copyright (C) 1996-2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/vi_FindDialogBox.h"

#include <utility>

#include "base/logging.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/regex_matcher.h"
#include "evita/text/search_and_replace_model.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

#define BEGIN_COMMAND_MAP switch (wParam) {
#define END_COMMAND_MAP } return DialogBox::onCommand(wParam, lParam);

#define ON_COMMAND(mp_ctrl, mp_notify, mp_method, ...) \
  case MAKEWPARAM(mp_ctrl, mp_notify): { \
    mp_method(__VA_ARGS__); \
    return true; \
  }

namespace {

// Replaces region with specified case.
void CaseReplace(text::Range* range, StringCase string_case) {
  switch (string_case) {
    case StringCase_Capitalized:
      range->Capitalize();
      break;

    case StringCase_CapitalizedAll:
      range->CapitalizeAll();
      break;

    case StringCase_Lower:
      range->Downcase();
      break;

    case StringCase_Mixed:
    case StringCase_None:
      // Nothing to do
      break;

    case StringCase_Upper:
      range->Upcase();
      break;
  }
}

Selection* GetActiveSelection() {
  auto const edit_pane = Application::instance()->GetActiveFrame()->
    GetActivePane()->as<EditPane>();

  if (!edit_pane)
    return nullptr;

  auto const window = edit_pane->GetActiveWindow();
  if (!window)
    return nullptr;

  if (auto const text_edit_window = window->as<TextEditWindow>())
    return text_edit_window->GetSelection();

  return nullptr;
}
}   // namespace

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
FindDialogBox::FindDialogBox(DialogBoxId dialog_box_id)
    : DialogBox(dialog_box_id),
      direction_(text::kDirectionDown),
      replace_in_(text::kReplaceInWhole) {
}

FindDialogBox::~FindDialogBox() {
}

void FindDialogBox::ClearMessage() {
  Application::instance()->GetActiveFrame()->ShowMessage(
    MessageLevel_Information, base::string16());
}

void FindDialogBox::DoFind(text::Direction eDirection) {
  UI_DOM_AUTO_LOCK_SCOPE();
  ClearMessage();

  direction_ = eDirection;

  text::SearchParameters search;
  auto const selection = PrepareFind(&search);
  if (!selection) {
    // We may not have search text.
    return;
  }

  if (text::kDirectionUp == eDirection)
    search.flags |= text::SearchFlag_Backward;

  auto const buffer = selection->GetBuffer();
  auto start_position = selection->GetStart();
  auto end_position = selection->GetEnd();
  if (!search.IsInSelection()){
    if (search.IsBackward()) {
      start_position = 0;
      end_position = selection->GetStart();
    } else {
      start_position = selection->GetEnd();
      end_position = buffer->GetEnd();
    }
  }

  text::RegexMatcher matcher(&search, selection->GetBuffer(), start_position,
                             end_position);

  auto const error_info = matcher.GetError();
  if (auto const error_code = error_info.error_code) {
    Application::instance()->GetActiveFrame()->ShowMessage(
        MessageLevel_Warning,
        IDS_BAD_REGEX,
        error_info.offset, error_code);
    return;
  }

  if (!FindFirst(&matcher)) {
    ReportNotFound();
    return;
  }

  selection->SetRange(matcher.GetMatched(0));
  selection->SetStartIsActive(search.IsBackward());
  selection->GetWindow()->MakeSelectionVisible();

  UpdateUI();
}

void FindDialogBox::DoReplace(text::ReplaceMode replace_mode) {
  UI_DOM_AUTO_LOCK_SCOPE();
  ClearMessage();

  text::SearchParameters search;
  auto const selection = PrepareFind(&search);
  if (!selection) {
    // We may not have search text.
    return;
  }

  auto const buffer = selection->GetBuffer();
  auto start_position = selection->GetStart();
  auto end_position = selection->GetEnd();
  if (!search.IsInSelection()) {
    start_position = buffer->GetStart();
    end_position = buffer->GetEnd();
  }

  text::RegexMatcher matcher(&search, selection->GetBuffer(), start_position,
                             end_position);

  if (!FindFirst(&matcher)) {
    ReportNotFound();
    return;
  }

  auto const wszWith = GetDlgItemText(IDC_FIND_WITH);
  bool is_replace_with_meta = search.flags & text::SearchFlag_Regex;

  Count num_replaced = 0;

  if (replace_mode == text::kReplaceOne) {
    auto range = matcher.GetMatched(0);
    if (selection->GetStart() == range->GetStart() &&
      selection->GetEnd() == range->GetEnd()) {
      if (search.IsCasePreserve()) {
        text::UndoBlock oUndo(buffer, L"Edit.Replace");
        StringCase eCase = selection->AnalyzeCase();
        matcher.Replace(wszWith, is_replace_with_meta);
        CaseReplace(selection, eCase);
      } else {
        matcher.Replace(wszWith, is_replace_with_meta);
      }
      ++num_replaced;
    }

    // Just select matched string or replaced string.
    selection->SetRange(range);
    selection->SetStartIsActive(false);
  } else {
    // Replace multiple matched strings
    text::UndoBlock oUndo(buffer, L"Edit.Replace");

    text::Range oRange(buffer, start_position, end_position);

    do {
      auto const range = matcher.GetMatched(0);
      DCHECK(range);

      bool fEmptyMatch = range->GetStart() == range->GetEnd();

      if (search.IsCasePreserve()) {
        StringCase eCase = range->AnalyzeCase();
        matcher.Replace(wszWith, is_replace_with_meta);
        CaseReplace(range, eCase);
      } else {
        matcher.Replace(wszWith, is_replace_with_meta);
      }

      ++num_replaced;

      if (fEmptyMatch) {
        auto const position = range->GetEnd();
        if (buffer->GetEnd() == position) {
          // We reach at end of buffer.
          break;
        }
        range->SetEnd(position + 1);
      }

      // FIXME 2008-07-09 yosi@msn.com We should allow interrupt
      // replacing.
      range->Collapse(Collapse_End);
    } while (matcher.NextMatch());
  }

  Application::instance()->GetActiveFrame()->ShowMessage(
      MessageLevel_Information,
      IDS_REPLACED,
      num_replaced);

  selection->GetWindow()->MakeSelectionVisible();
}

/// <summary>
///   Find the first match by executing matcher.
/// </summary>
/// <param name="pMatcher">
///   A matcher initialized from Find Dialog input elements.
/// </param>
bool FindDialogBox::FindFirst(text::RegexMatcher* pMatcher) {
  if (pMatcher->FirstMatch())
    return true;

  if (pMatcher->WrapMatch()) {
    Application::instance()->ShowMessage(MessageLevel_Information,
                                         IDS_PASSED_END);
    return true;
  }

  return false;
}

/// <summary>
///   Handles [Cancel] button.
/// </summary>
void FindDialogBox::onCancel() {
  ClearMessage();
  ::ShowWindow(*this, SW_HIDE);
  ::SetActiveWindow(*Application::instance()->GetActiveFrame());
}

/// <summary>
///   Dispatch WM_COMMAND to controls.
/// </summary>
/// <param name="wParam">
///   wParam of window message
/// </param>
/// <param name="lParam">
///   lParam of window message
/// </param>
bool FindDialogBox::onCommand(WPARAM wParam, LPARAM lParam) {
  DVLOG(1) << "FindDialogBox::onCommand ctrlid=" << LOWORD(wParam) <<
      " notify=" << std::hex << HIWORD(wParam);

  BEGIN_COMMAND_MAP
    ON_COMMAND(IDOK, BN_CLICKED, onOk) // [Entery] key
    ON_COMMAND(IDCANCEL, BN_CLICKED, onCancel) // [Esc] key

    ON_COMMAND(IDC_FIND_NEXT, BN_CLICKED, onFindNext)
    ON_COMMAND(IDC_FIND_PREVIOUS, BN_CLICKED, onFindPrevious)
    ON_COMMAND(IDC_FIND_REPLACE, BN_CLICKED, onReplaceOne)
    ON_COMMAND(IDC_FIND_REPLACE_ALL, BN_CLICKED, onReplaceAll)
    ON_COMMAND(IDC_FIND_WHAT, CBN_EDITCHANGE, UpdateUI)
    ON_COMMAND(IDC_FIND_WITH, CBN_EDITCHANGE, UpdateUI)
  END_COMMAND_MAP
}

/// <summary>
///   Initialize Find dialog box. This method is called at WM_INITDIALOG.
/// </summary>
bool FindDialogBox::onInitDialog() {
  auto const dwExStyle = ::GetWindowLong(*this, GWL_EXSTYLE) |
      WS_EX_LAYERED;
  ::SetWindowLong(*this, GWL_EXSTYLE, dwExStyle);

 // FIXME 2007-08-20 yosi@msn.com We should get default value of
 // find dialog transparency.
  ::SetLayeredWindowAttributes(
      *this,
      RGB(0, 0, 0),
      80 * 255 / 100,
      LWA_ALPHA);

  SetCheckBox(IDC_FIND_EXACT, true);

 // FIXME 2007-08-20 yosi@msn.com We should get default value of
 // case-preserving replace.
  SetCheckBox(IDC_FIND_PRESERVE, true);

  UpdateUI(true);

  // Returns false not to set focus to dialog.
  return false;
}

/// <summary>
///   Dispatch windows message.
/// </summary>
INT_PTR FindDialogBox::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
 //DEBUG_PRINTF("%p uMsg=0x%04x\n", this, uMsg);
  switch (uMsg) {
    case WM_ACTIVATE:
      DVLOG(1) << "WM_ACTIVATE wParam=" << wParam;

      if (WA_INACTIVE == wParam) {
        //::SetActiveWindow(*Application::instance()->GetActiveFrame());
        //::SetFocus(*Application::instance()->GetActiveFrame());
        return FALSE;
      }

      UpdateUI(true);
      ::SetFocus(GetDlgItem(IDC_FIND_WHAT));
      return TRUE;

    case WM_WINDOWPOSCHANGED: {
      auto const wp = reinterpret_cast<WINDOWPOS*>(lParam);
      if (wp->flags & SWP_HIDEWINDOW) {
        DVLOG(0) << "WM_WINDOWPOSCHANGED: SWP_HIDEWINDOW";
        //::SetActiveWindow(*Application::instance()->GetActiveFrame());
        Application::instance()->GetActiveFrame()->GetActivePane()->SetFocus();
        return TRUE;
      }
      break;
    }
  }
  return DialogBox::onMessage(uMsg, wParam, lParam);
}

void FindDialogBox::onOk() {
  switch (direction_) {
    case text::kDirectionUp:
      onFindPrevious();
      break;
    case text::kDirectionDown:
      onFindNext();
      break;
  }
}

void FindDialogBox::onFindNext() {
  DoFind(text::kDirectionDown);
}

void FindDialogBox::onFindPrevious() {
  DoFind(text::kDirectionUp);
}

void FindDialogBox::onReplaceAll() {
  DoReplace(text::kReplaceAll);
}

void FindDialogBox::onReplaceOne() {
  DoReplace(text::kReplaceOne);
}

/// <summary>
///   Preparation for search. We extract search parameters from dialog box.
/// </summary>
/// <returns>Selection to start search.</returns>
Selection* FindDialogBox::PrepareFind(text::SearchParameters* search) {
  DCHECK(search);
  auto const selection = GetActiveSelection();
  if (!selection) {
    // Active pane isn't editor.
    return nullptr;
  }

  search->search_text = std::move(GetDlgItemText(IDC_FIND_WHAT));
  if (search->search_text.empty())
    return nullptr;
  search->flags = 0;

  if (!GetChecked(IDC_FIND_CASE))  {
    for (auto const ch : search->search_text) {
      if (IsLowerCase(ch)) {
        search->flags |= text::SearchFlag_IgnoreCase;
      } else if (IsUpperCase(ch)) {
        search->flags &= ~text::SearchFlag_IgnoreCase;
        break;
      }
    }
  }

  if (GetChecked(IDC_FIND_PRESERVE)) {
    auto const wszWith = GetDlgItemText(IDC_FIND_WITH);
    for (auto ch : wszWith) {
      if (IsLowerCase(ch)) {
        search->flags |= text::SearchFlag_CasePreserve;
      } else if (IsUpperCase(ch)) {
        search->flags &= ~text::SearchFlag_CasePreserve;
        break;
      }
    }
  }

  if (GetChecked(IDC_FIND_REGEX))
    search->flags |= text::SearchFlag_Regex;

  if (GetChecked(IDC_FIND_WORD))
    search->flags |= text::SearchFlag_MatchWord;

  if (GetChecked(IDC_FIND_SELECTION))
    search->flags |= text::SearchFlag_InSelection;

  return selection;
}

void FindDialogBox::ReportNotFound() {
  Application::instance()->GetActiveFrame()->ShowMessage(
      MessageLevel_Information, IDS_NOT_FOUND);
}

/// <summary>Updates find dialog box controls.</summary>
/// <param name="fActivate">True if dialog box is activated</param>
void FindDialogBox::UpdateUI(bool fActivate) {
  SetCheckBox(IDC_FIND_DOWN, text::kDirectionDown == direction_);
  SetCheckBox(IDC_FIND_UP, text::kDirectionUp == direction_);

  auto const cwch = ::GetWindowTextLength(GetDlgItem(IDC_FIND_WHAT));

  ::EnableWindow(GetDlgItem(IDC_FIND_NEXT), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_PREVIOUS), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_WITH), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE), cwch >= 1);
  ::EnableWindow(GetDlgItem(IDC_FIND_REPLACE_ALL), cwch >= 1);

 // If active selection covers mutliple lines, Search/Replace can be
 // limited in selection.
  if (auto const selection = GetActiveSelection()) {
    auto const fHasNewline = selection->FindFirstChar('\n') >= 0;
    ::EnableWindow(GetDlgItem(IDC_FIND_SELECTION), fHasNewline);
    ::EnableWindow(GetDlgItem(IDC_FIND_WHOLE_FILE), fHasNewline);

    if (fActivate) {
      replace_in_ = fHasNewline ? text::kReplaceInSelection :
                                  text::kReplaceInWhole;
    }
    SetCheckBox(IDC_FIND_SELECTION, text::kReplaceInSelection == replace_in_);
    SetCheckBox(IDC_FIND_WHOLE_FILE, text::kReplaceInWhole == replace_in_);
  }
}
