//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - visual - text pane
// listener/winapp/vi_text_pane.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_TextEditWindow.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_text_pane_h)
#define INCLUDE_listener_winapp_visual_text_pane_h

#include "evita/content/content_window.h"
#include "./li_util.h"
#include "./gfx_base.h"
#include "./vi_CommandWindow.h"
#include "./vi_Page.h"
#include <memory>

class Caret;
class EditPane;
class Page;
class Selection;

namespace dom {
class Document;
}

using Buffer = dom::Document;

enum DragMode
{
    DragMode_None,
    DragMode_Selection,
}; // DragMode

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
// Member Variables:
// m_pViewRange
// A range contains the start position of window.
//
class TextEditWindow
    : public CommandWindow_<TextEditWindow, content::ContentWindow>,
      public DoubleLinkedNode_<TextEditWindow, Buffer> {
  DECLARE_CASTABLE_CLASS(TextEditWindow, content::ContentWindow);

  private: typedef DoubleLinkedNode_<TextEditWindow> WindowItem;
  private: typedef content::ContentWindow ParentClass;

  private: typedef Edit::Range Range;

  private: struct ScrollBar {
    HWND m_hwnd;
    int m_nBar;

    ScrollBar()
        : m_hwnd(nullptr),
          m_nBar(SB_CTL) {
    }

    bool GetInfo(SCROLLINFO* pInfo) {
      return m_hwnd && ::GetScrollInfo(m_hwnd, m_nBar, pInfo);
    }

    HWND GetHwnd() const { return m_hwnd; }

    void Set(HWND hwnd, int nBar) {
      m_hwnd = hwnd;
      m_nBar = nBar;
    }

    void ShowWindow(int) const;

    void SetInfo(SCROLLINFO* pInfo, bool fRedraw) {
      if (!m_hwnd)
        return;
      ::SetScrollInfo(m_hwnd, m_nBar, pInfo, fRedraw);
    }
  };

  private: class Autoscroller;
  private: class CaretBlinker;

  private: std::unique_ptr<Autoscroller> autoscroller_;
  private: std::unique_ptr<Caret> caret_;
  private: std::unique_ptr<CaretBlinker> caret_blinker_;
  private: DragMode m_eDragMode;
  private: const gfx::Graphics* m_gfx;
  private: Posn m_lCaretPosn;
  private: int m_nCharTick;
  private: ScrollBar m_oHoriScrollBar;
  private: ScrollBar m_oVertScrollBar;
  // TODO(yosi): Manage life time of selection.
  private: Selection* selection_;
  #if SUPPORT_IME
  private: bool m_fImeTarget;
  private: Posn m_lImeStart;
  private: Posn m_lImeEnd;
  #endif // SUPPORT_IME
  private: Range* m_pViewRange;
  private: std::unique_ptr<Page> m_pPage;

  // ctor/dtor
  public: TextEditWindow(Buffer*, Posn = 0);
  public: virtual ~TextEditWindow();

  // [B]
  public: void Blink(Posn, int);

  // [C]
  public: TextEditWindow* Clone() const;
  public: Count ComputeMotion(Unit, Count, const gfx::PointF&, Posn*);
  private: Posn computeGoalX(float, Posn);

  // [D]
  public: void DidChangeFrame();
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: virtual void DidShow() override;

  // [E]
  public: Posn EndOfLine(Posn);
  private: Posn endOfLineAux(const gfx::Graphics&, Posn);

  // [F]
  private: void ForceRedraw();
  private: void ForceRedrawLater();
  private: void format(const gfx::Graphics&, Posn);

  // [G]
  public: Buffer* GetBuffer() const;
  public: HCURSOR GetCursorAt(const Point&) const;
  // TODO(yosi): We should not expose TextEdintWindow::GetTitle(). We export
  // this for EditPane.
  public: virtual base::string16 GetTitle(size_t max_length) const override;

  public: static const char* GetClass_() { return "TextEditWindow"; }

  public: Count GetColumn(Posn);
  public: Posn GetEnd();

  public: HWND GetScrollBarHwnd(int) const;
  public: Selection* GetSelection() const { return &*selection_; }
  public: Posn GetStart();
  public: size_t GetUndoSize() const;

  // [L]
  public: int LargeScroll(int, int, bool = true);

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) override;
  public: virtual void MakeSelectionVisible() override;
  public: Posn MapPointToPosn(const gfx::PointF point);
  public: gfx::RectF MapPosnToPoint(Posn);

  // [O]
  private: virtual bool OnIdle(uint) override;
  private: virtual LRESULT OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam);
  private: virtual void OnLeftButtonDown(uint, const Point&) override;
  private: virtual void OnLeftButtonUp(uint, const Point&) override;
  private: virtual void OnMouseMove(uint, const Point&) override;
  private: virtual void OnPaint(const gfx::Rect) override;
  private: void onVScroll(uint);

  // [R]
  public: virtual void Redraw() override;
  private: void Render();

  // [S]
  private: void selectWord(Posn);
  public: void SetScrollBar(HWND, int);
  public: int SmallScroll(int, int);
  public: Posn StartOfLine(Posn);
  private: Posn startOfLineAux(const gfx::Graphics&, Posn);
  private: void stopDrag();

  // [U]
  private: void updateScreen();
  private: void updateScrollBar();
  // TODO(yosi): We should not expose TextEdintWindow::UpdateStatusBar(). We
  // export this for EditPane.
  public: virtual void UpdateStatusBar() const override;

  // [W]
  private: virtual void WillDestroyWidget() override;

  #if SUPPORT_IME
  private: void onImeComposition(LPARAM);
  public: void Reconvert(Posn, Posn);
  private: uint setReconvert(RECONVERTSTRING*, Posn, Posn);
  private: BOOL showImeCaret(SIZE, POINT);
  #endif // SUPPORT_IME

  DISALLOW_COPY_AND_ASSIGN(TextEditWindow);
};

typedef DoubleLinkedList_<TextEditWindow, Buffer> WindowList;

#endif //!defined(INCLUDE_listener_winapp_visual_text_pane_h)
