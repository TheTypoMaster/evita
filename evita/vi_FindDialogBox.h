// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_find_dialog_box_h)
#define INCLUDE_evita_find_dialog_box_h

#include "./IStringCursor.h"
#include "./resource.h"
#include "./vi_DialogBox.h"

class RegexMatcher;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
class FindDialogBox final : public DialogBox {
  public: enum Direction {
    kDirectionDown,
    kDirectionUp,
  };

  public: enum ReplaceIn {
    kReplaceInSelection,
    kReplaceInWhole,
  };

  private: enum ReplaceMode {
    kReplaceAll,
    kReplaceOne,
  };

  private: Direction direction_;
  private: ReplaceIn replace_in_;

  public: FindDialogBox();
  public: virtual ~FindDialogBox();

  private: void ClearMessage();
  public: void DoFind(Direction);
  private: void DoReplace(ReplaceMode replace_mode);
  private: bool FindFirst(RegexMatcher* matcher);
  private: void onFindNext();
  private: void onFindPrevious();
  private: void onReplaceOne();
  private: void onReplaceAll();
  private: Selection* PrepareFind(SearchParameters*);
  private: void ReportNotFound();
  private: void UpdateUI(bool activate = false);

  // DialogBox
  private: virtual int GetTemplate() const override { return IDD_FIND; }
  private: virtual bool onInitDialog() override;
  private: virtual void onCancel() override;
  private: virtual bool onCommand(WPARAM, LPARAM) override;
  private: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM) override;
  private: virtual void onOk() override;
};

#endif //!defined(INCLUDE_evita_find_dialog_box_h)
