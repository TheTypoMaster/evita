//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Edit Pane
// listener/winapp/vi_EditPane.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_EditPane.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_EditPane_h)
#define INCLUDE_listener_winapp_visual_EditPane_h

#include "base/memory/ref_counted.h"
#include "evita/vi_Pane.h"

class Frame;

namespace text {
class Buffer;
}

namespace ui {
class MouseEvent;
}

namespace views {
class ContentWindow;
}

// EditPane is a container of multiple ContentWindow windows and layouts
// them vertically with draggable splitter.
class EditPane final : public Pane {
  DECLARE_CASTABLE_CLASS(EditPane, Pane);

  private: typedef common::win::Point Point;
  private: typedef common::win::Rect Rect;

  private: enum Limits {
    k_cxSplitter = 8,
    k_cxSplitterBig = 11,
    k_cxMinBox = 50,
    k_cySplitter = 8,
    k_cySplitterBig = 11,
    k_cyMinBox = k_cySplitter,
  };

  private: typedef views::ContentWindow Window;

  private: class Box;
  private: class LayoutBox;
  private: class LeafBox;
  private: class HorizontalLayoutBox;
  private: class VerticalLayoutBox;
  private: struct HitTestResult;
  private: class SplitterController;

  private: enum State {
    State_NotRealized,
    State_Destroyed,
    State_Realized,
  };

  private: State m_eState;
  private: scoped_refptr<LayoutBox> root_box_;
  private: const std::unique_ptr<SplitterController> splitter_controller_;

  // ctro/dtor
  public: explicit EditPane(Window* window);
  public: virtual ~EditPane();

  public: Frame& frame() const;

  public: bool has_more_than_one_child() const {
    return first_child() != last_child();
  }

  // [A]
  public: virtual void Activate() override;

  // [D]
  private: virtual void DidRealize() override;
  private: virtual void DidRealizeChildWidget(const Widget&) override;
  private: virtual void DidRemoveChildWidget(const Widget&) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;

  // [G]
  private: LeafBox* GetActiveLeafBox() const;
  private: virtual HCURSOR GetCursorAt(const Point&) const override;
  public: Window* GetActiveWindow() const;
  public: virtual views::Window* GetWindow() const override;

  public: Window* GetFirstWindow() const;
  public: Window* GetLastWindow() const;

  // [I]
  public: bool IsRealized() const {
    return m_eState == State_Realized;
  }

  // [O]
  private: virtual void OnDraw(gfx::Canvas* gfx) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  // [R]
  public: void ReplaceActiveWindow(Window* window);

  // [S]
  public: void SplitHorizontally(Window* left_window,
                                 Window* new_right_window);
  public: void SplitVertically(Window* above_window,
                               Window* new_below_window);

  // [W]
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& child) override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_listener_winapp_visual_EditPane_h)
