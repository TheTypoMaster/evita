#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Edit Pane
// listener/winapp/vi_EditPane.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_EditPane.cpp#3 $
//
#define DEBUG_REDRAW 0
#define DEBUG_RESIZE 0
#define DEBUG_SPLIT 0
#include "./vi_EditPane.h"

#include "base/logging.h"
#include "content/content_window.h"
#include "./ed_Mode.h"
#include "./gfx_base.h"
#include "evita/editor/application.h"
#include "./vi_Buffer.h"
#include "./vi_Selection.h"
#include "./vi_TextEditWindow.h"
#include "./vi_util.h"

struct EditPane::HitTestResult {
  enum Type {
    None,
    HScrollBar,
    HSplitter,
    HSplitterBig,
    VScrollBar,
    VSplitter,
    VSplitterBig,
    Window,
  };

  Box* box;
  Type type;

  HitTestResult() : box(nullptr), type(None) {
  }

  HitTestResult(Type type, const Box& box)
      : box(const_cast<Box*>(&box)), type(type) {
    ASSERT(type != None);
  }

  content::ContentWindow* window() const;
};

class EditPane::Box : public DoubleLinkedNode_<EditPane::Box>,
                      public RefCounted_<EditPane::Box> {
    protected: EditPane* edit_pane_;
    private: bool is_removed_;
    private: LayoutBox* outer_;
    private: Rect rect_;
    protected: Box(EditPane*, LayoutBox*);
    public: virtual ~Box();
    public: bool is_removed() const { return is_removed_; }
    public: virtual Box* first_child() const { return nullptr; }
    public: Box* next_sibling() const { return GetNext(); }
    public: Box* previous_sibling() const { return GetPrev(); }
    public: LayoutBox* outer() const { return outer_; }
    public: const gfx::Rect& rect() const { return rect_; }
    public: Rect& rect() { return rect_; }
    public: void set_outer(LayoutBox& outer) { outer_ = &outer; }
    public: virtual void CloseAllBut(Window*) = 0;
    public: virtual uint CountLeafBox() const = 0;
    public: virtual void Destroy() = 0;
    public: virtual void DrawSplitters(const gfx::Graphics&) { }
    public: virtual LeafBox* FindLeafBoxFromWidget(
        const Widget&) const = 0;
    public: virtual LeafBox* GetActiveLeafBox() const = 0;
    public: virtual LeafBox* GetFirstLeafBox() const = 0;

    public: virtual HitTestResult HitTest(Point) const = 0;

    public: virtual bool IsLeafBox() const = 0;

    public: virtual void Realize(EditPane*, const gfx::Rect&);
    public: virtual void Redraw() const {}
    public: void Removed();

    public: virtual void SetRect(const gfx::Rect&);

    DISALLOW_COPY_AND_ASSIGN(Box);
};

class EditPane::LayoutBox : public EditPane::Box {
  protected: typedef DoubleLinkedList_<Box> BoxList;
  protected: BoxList boxes_;
  protected: LayoutBox(EditPane*, LayoutBox*);
  public: virtual ~LayoutBox();
  public: virtual Box* first_child() const override final {
    return boxes_.GetFirst();
  }
  public: void Add(Box& box);
  public: virtual void CloseAllBut(Window*) override final;
  public: virtual uint CountLeafBox() const override final;
  public: virtual void Destroy() override final;
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) = 0;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) = 0;
  public: virtual LeafBox* GetActiveLeafBox() const override final;
  public: virtual LeafBox* GetFirstLeafBox() const override final;
  public: virtual LeafBox* FindLeafBoxFromWidget(
      const Widget&) const override final;

  public: virtual bool IsLeafBox() const override final { return false; }
  public: bool IsSingle() const;
  public: virtual bool IsVerticalLayoutBox() const = 0;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void Redraw() const override final;
  public: void RemoveBox(Box&);
  public: void Replace(Box&, Box&);
  public: virtual LeafBox& Split(Box&, int) = 0;

  public: virtual void StopSplitter(const gfx::Point&, Box&) = 0;

  // [U]
  public: void UpdateSplitters();

  DISALLOW_COPY_AND_ASSIGN(LayoutBox);
};

class EditPane::LeafBox final : public EditPane::Box {
  private: HWND m_hwndVScrollBar;
  private: Window* m_pWindow;

  public: LeafBox(EditPane* edit_pane, LayoutBox* outer, Window* pWindow)
    : Box(edit_pane, outer),
      m_hwndVScrollBar(nullptr),
      m_pWindow(pWindow) {
  }

  public: virtual ~LeafBox();

  // [C]
  public: virtual void CloseAllBut(Window*) override;
  public: virtual uint CountLeafBox() const  override final { return 1; }

  // [D]
  public: virtual void Destroy() override;
  public: void DetachWindow();

  public: void EnsureInHorizontalLayoutBox();
  public: void EnsureInVerticalLayoutBox();

  // [G]
  public: virtual LeafBox* GetActiveLeafBox() const override;
  public: virtual LeafBox* GetFirstLeafBox() const override;
  public: virtual LeafBox* FindLeafBoxFromWidget(
      const Widget&) const override;
  public: Window* GetWindow() const { return m_pWindow; }

  // [H]
  private: bool HasSibling() const { return GetNext() || GetPrev(); }
  public: virtual HitTestResult HitTest(Point) const override;

  // [I]
  public: virtual bool IsLeafBox() const override final { return true; }

  // [R]
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void Redraw() const override;
  public: void ReplaceWindow(Window* window);

  // [S]
  public: virtual void SetRect(const gfx::Rect&) override;

  DISALLOW_COPY_AND_ASSIGN(LeafBox);
};

class EditPane::HorizontalLayoutBox final : public EditPane::LayoutBox {
  public: HorizontalLayoutBox(EditPane*, LayoutBox*);
  public: virtual ~HorizontalLayoutBox();
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) override;
  public: virtual HitTestResult HitTest(Point) const override;
  public: virtual void DrawSplitters(const gfx::Graphics&) override;
  public: virtual bool IsVerticalLayoutBox() const override;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) override;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void SetRect(const gfx::Rect&) override;
  public: virtual LeafBox& Split(Box&, int) override;
  public: virtual void StopSplitter(const gfx::Point&, Box&) override;
  DISALLOW_COPY_AND_ASSIGN(HorizontalLayoutBox);
};

class EditPane::VerticalLayoutBox final : public LayoutBox {
  public: VerticalLayoutBox(EditPane*, LayoutBox*);
  public: virtual ~VerticalLayoutBox();
  protected: virtual void DidRemoveBox(Box*, Box*, const gfx::Rect&) override;
  public: virtual HitTestResult HitTest(Point) const override;
  public: virtual void DrawSplitters(const gfx::Graphics&) override;
  public: virtual bool IsVerticalLayoutBox() const override;
  public: virtual void MoveSplitter(const gfx::Point&, Box&) override;
  public: virtual void Realize(EditPane*, const gfx::Rect&) override;
  public: virtual void SetRect(const gfx::Rect&) override;
  public: virtual LeafBox& Split(Box&, int) override;
  public: virtual void StopSplitter(const gfx::Point&, Box&) override;
  DISALLOW_COPY_AND_ASSIGN(VerticalLayoutBox);
};

namespace {
class StockCursor {
  private: HCURSOR hCursor_;
  private: HINSTANCE hInstance_;
  private: const char16* id_;
  public: explicit StockCursor(int id)
    : StockCursor(g_hInstance, MAKEINTRESOURCE(id)) {
  }
  public: explicit StockCursor(const char16* id)
    : StockCursor(nullptr, id) {
  }
  private: StockCursor(HINSTANCE instance, const char16* id)
      : hCursor_(nullptr), hInstance_(instance), id_(id) {
  }
  public: operator HCURSOR() {
    if (!hCursor_) {
      hCursor_ = ::LoadCursor(hInstance_, id_);
      ASSERT(hCursor_);
    }
    return hCursor_;
  }
  DISALLOW_COPY_AND_ASSIGN(StockCursor);
};

void DrawSplitter(const gfx::Graphics& gfx, RECT* prc,
                         uint /*grfFlag*/) {
  auto rc = *prc;
  //gfx::Brush fillBrush(gfx, gfx::sysColor(COLOR_3DFACE));
  gfx::Brush fillBrush(gfx, gfx::ColorF(gfx::ColorF::Pink));
  gfx.FillRectangle(fillBrush, rc);
  //::DrawEdge(gfx, &rc, EDGE_RAISED, grfFlag);
}
} // namesapce

//
// EditPane
//

EditPane::Box::Box(EditPane* edit_pane, LayoutBox* outer)
    : edit_pane_(edit_pane),
      is_removed_(false),
      outer_(outer) {}

EditPane::Box::~Box() {
  ASSERT(is_removed_);
  ASSERT(!outer_);
  ASSERT(!GetNext());
  ASSERT(!GetPrev());
}

void EditPane::Box::Realize(EditPane*, const gfx::Rect& rect) {
  rect_ = rect;
}

void EditPane::Box::Removed() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!is_removed());
  is_removed_ = true;
  outer_ = nullptr;
}

void EditPane::Box::SetRect(const gfx::Rect& rect) {
  rect_ = rect;
}

// HorizontalLayoutBox
EditPane::HorizontalLayoutBox::HorizontalLayoutBox(EditPane* edit_pane,
                                                   LayoutBox* outer)
    : LayoutBox(edit_pane, outer) {
}

EditPane::HorizontalLayoutBox::~HorizontalLayoutBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
}

void EditPane::HorizontalLayoutBox::DidRemoveBox(
    Box* const pAbove,
    Box* const pBelow,
    const gfx::Rect& rc) {
  if (pAbove) {
    // Extend pane above.
    RECT rect = pAbove->rect();
    rect.right = rc.right;
    pAbove->SetRect(rect);
  } else if (pBelow) {
    // Extend pane below.
    RECT rect = pBelow->rect();
    rect.left = rc.left;
    pBelow->SetRect(rect);
  }
}

void EditPane::HorizontalLayoutBox::DrawSplitters(const gfx::Graphics& gfx) {
  if (boxes_.GetFirst() == boxes_.GetLast()) {
    return;
  }

  auto rc = rect();
  foreach (BoxList::Enum, it, boxes_) {
    auto const box = it.Get();
    box->DrawSplitters(gfx);
    if (auto const right_box = box->GetPrev()) {
      rc.left = right_box->rect().right;
      rc.right = box->rect().left;
      DrawSplitter(gfx, &rc, BF_LEFT | BF_RIGHT);
    }
  }
}

EditPane::HitTestResult EditPane::HorizontalLayoutBox::HitTest(
    Point pt) const {
  if (!::PtInRect(&rect(), pt)) {
    return HitTestResult();
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const result = it->HitTest(pt);
    if (result.type != HitTestResult::None) {
      return result;
    }

    if (auto const left_box = it->GetPrev()) {
      RECT splitterRect;
      splitterRect.top = rect().top;
      splitterRect.bottom = rect().bottom;
      splitterRect.left = left_box->rect().right;
      splitterRect.right = it->rect().left;
      if (::PtInRect(&splitterRect, pt)) {
        return HitTestResult(HitTestResult::HSplitter, *it.Get());
      }
    }
  }

  return HitTestResult();
}

bool EditPane::HorizontalLayoutBox::IsVerticalLayoutBox() const {
  return false;
}

void EditPane::HorizontalLayoutBox::MoveSplitter(
    const gfx::Point& pt,
    Box& right_box) {
  auto& left_box = right_box.GetPrev()
      ? *right_box.GetPrev()
      : Split(right_box, k_cxSplitter);

  if (pt.x - left_box.rect().left <= 0) {
    // Above box is too small.
  } else if (right_box.rect().right - pt.x <= k_cxMinBox) {
    // Below box is too small.
  } else {
    left_box.rect().right = pt.x;
    right_box.rect().left = pt.x + k_cxSplitter;
    left_box.SetRect(left_box.rect());
    right_box.SetRect(right_box.rect());
  }

  UpdateSplitters();
}

void EditPane::HorizontalLayoutBox::Realize(
    EditPane* edit_pane,
    const gfx::Rect& rect) {
  LayoutBox::Realize(edit_pane, rect);

  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->Realize(edit_pane, rect);
    return;
  }

  auto const width = rect.right - rect.left;
  auto const content_width = width - k_cxSplitter * (num_boxes - 1);
  auto const box_width = content_width / num_boxes;
  RECT elemRect(rect);
  foreach (BoxList::Enum, it, boxes_) {
    elemRect.right = rect.left + box_width;
    it->Realize(edit_pane, elemRect);
    elemRect.left = elemRect.right + k_cxSplitter;
  }
}

void EditPane::HorizontalLayoutBox::SetRect(const gfx::Rect& newRect) {
  RECT rcOld = rect();
  LayoutBox::SetRect(newRect);
  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->SetRect(newRect);
    return;
  }

  auto const cxNewPane = rect().right  - rect().left;
  auto const cxOldPane = rcOld.right - rcOld.left;

  if (!cxOldPane) {
    auto const cBoxes = boxes_.Count();
    if (!cBoxes) {
      return;
    }

    auto const cxNewWin = cxNewPane / cBoxes;
    auto xBox = rect().left;
    auto cxSplitter = 0;
    auto pBox = static_cast<Box*>(nullptr);
    foreach (BoxList::Enum, oEnum, boxes_) {
      pBox = oEnum.Get();
      auto const prc = &pBox->rect();
      xBox += cxSplitter;
      prc->left = xBox;
      xBox += cxNewWin;
      prc->right = xBox;
      cxSplitter = k_cxSplitter;
    }

    if (pBox) {
      pBox->rect().right = rect().right;
    }
  } else {
    ScopedRefCount_<LayoutBox> protect(*this);

    tryAgain:
      auto xBox = rect().left;
      auto cxSplitter = 0;
      auto pBox = static_cast<Box*>(nullptr);
      foreach (BoxList::Enum, oEnum, boxes_) {
        pBox = oEnum.Get();
        auto const prc = &pBox->rect();
        auto const cxOldWin = prc->right - prc->left;
        auto const cxNewWin = cxNewPane * cxOldWin / cxOldPane;
        if (cxNewWin < k_cxMinBox) {
          pBox->Destroy();
          if (is_removed())
            return;
          goto tryAgain;
        }
        xBox += cxSplitter;
        prc->left = xBox;
        xBox += cxNewWin;
        prc->right = xBox;
        cxSplitter = k_cxSplitter;
      }

      if (!pBox) {
        return;
      }
      pBox->rect().right = rect().right;
  }

  foreach (BoxList::Enum, oEnum, boxes_) {
    auto const pBox = oEnum.Get();
    auto newRect = pBox->rect();
    newRect.top = rect().top;
    newRect.bottom = rect().bottom;
    pBox->SetRect(newRect);
  }
}

EditPane::LeafBox& EditPane::HorizontalLayoutBox::Split(
    Box& below_box,
    int cxBox) {
  auto pBelow = below_box.GetActiveLeafBox();
  auto const below = pBelow->GetWindow()->as<TextEditWindow>();
  if (!below)
    return *pBelow;
  auto const prcBelow = &pBelow->rect();
  ASSERT(prcBelow->right - prcBelow->left > cxBox);
  DCHECK_EQ(pBelow->GetWindow()->container_widget(), edit_pane_);

  auto const pWindow = new TextEditWindow(
      below->GetBuffer(),
      below->GetStart());
  edit_pane_->AppendChild(*pWindow);

  auto const pSelection = below->GetSelection();

  pWindow->GetSelection()->SetRange(
      pSelection->GetStart(),
      pSelection->GetEnd());

  pWindow->GetSelection()->SetStartIsActive(
      pSelection->IsStartActive());

  auto const pAbove = new LeafBox(edit_pane_, this, pWindow);
  auto const prcAbove = &pAbove->rect();

  boxes_.InsertBefore(pAbove, pBelow);
  pAbove->AddRef();

  prcAbove->top = prcBelow->top;
  prcAbove->bottom = prcBelow->bottom;
  prcAbove->left = prcBelow->left;
  prcAbove->right = prcBelow->left + cxBox;

  pAbove->Realize(edit_pane_, *prcAbove);

  prcBelow->left = prcAbove->right + k_cxSplitter;
  pBelow->SetRect(pBelow->rect());

  UpdateSplitters();

  return *pAbove;
}

void EditPane::HorizontalLayoutBox::StopSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  if (!below_box.GetPrev()) {
    return;
  }

  ScopedRefCount_<LayoutBox> protect(*this);
  auto& above_box = *below_box.GetPrev();
  auto const cxMin = k_cxMinBox;
  if (pt.x - above_box.rect().left < cxMin) {
    below_box.rect().left = above_box.rect().left;
    above_box.Destroy();
    below_box.SetRect(below_box.rect());
    UpdateSplitters();
  } else if (below_box.rect().right - pt.x < k_cxMinBox) {
    above_box.rect().right = below_box.rect().right;
    below_box.Destroy();
    above_box.SetRect(above_box.rect());
    UpdateSplitters();
  }
}

// LayoutBox
EditPane::LayoutBox::LayoutBox(EditPane* edit_pane, LayoutBox* outer)
    : Box(edit_pane, outer) {
}

EditPane::LayoutBox::~LayoutBox() {
  ASSERT(boxes_.IsEmpty());
}

void EditPane::LayoutBox::Add(Box& box) {
  ASSERT(!is_removed());
  boxes_.Append(&box);
  box.AddRef();
}

EditPane::LeafBox* EditPane::LayoutBox::GetActiveLeafBox() const {
  ASSERT(!is_removed());
  class Local {
    public: static LeafBox* SelectActiveBox(LeafBox* box1, LeafBox* box2) {
      return box1 && box2
          ? activeTick(*box1) > activeTick(*box2) ? box1 : box2
          : box1 ? box1 : box2;
    }

    private: static uint activeTick(const LeafBox& box) {
      auto& window = *box.GetWindow();
      return window.is_shown() ? window.GetActiveTick() : 0u;
    }
  };

  auto candiate = static_cast<LeafBox*>(nullptr);
  for (auto& box: boxes_) {
    auto const other = box.GetActiveLeafBox();
    //if (other && other->GetWindow()->is_shown())
    candiate = Local::SelectActiveBox(candiate, other);
  }
  return candiate;
}

EditPane::LeafBox* EditPane::LayoutBox::GetFirstLeafBox() const {
  ASSERT(!is_removed());
  return boxes_.GetFirst() ? boxes_.GetFirst()->GetFirstLeafBox() : nullptr;
}

void EditPane::LayoutBox::CloseAllBut(Window* window) {
  ASSERT(!is_removed());
  auto runner = boxes_.GetFirst();
  while (runner) {
    auto const next = runner->GetNext();
    runner->CloseAllBut(window);
    runner = next;
  }
}

uint EditPane::LayoutBox::CountLeafBox() const {
  ASSERT(!is_removed());
  auto count = 0u;
  foreach (BoxList::Enum, it, boxes_) {
    count += it->CountLeafBox();
  }
  return count;
}

void EditPane::LayoutBox::Destroy() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!is_removed());
  auto runner = boxes_.GetFirst();
  while (runner) {
    auto const next = runner->GetNext();
    runner->Destroy();
    runner = next;
  }
}

EditPane::LeafBox* EditPane::LayoutBox::FindLeafBoxFromWidget(
    const Widget& window) const {
  ASSERT(!is_removed());
  foreach (BoxList::Enum, it, boxes_) {
    if (auto const box = it->FindLeafBoxFromWidget(window)) {
      return box;
    }
  }
  return nullptr;
}

bool EditPane::LayoutBox::IsSingle() const {
  ASSERT(!is_removed());
  return !boxes_.IsEmpty() && boxes_.GetFirst() == boxes_.GetLast();
}

void EditPane::LayoutBox::Realize(EditPane* edit_pane, const gfx::Rect& rect) {
  ASSERT(!is_removed());
  Box::Realize(edit_pane, rect);
}

void EditPane::LayoutBox::Redraw() const {
  for (auto& box : boxes_) {
    box.Redraw();
  }
}

void EditPane::LayoutBox::RemoveBox(Box& box) {
  ASSERT(!is_removed());
  auto const pAbove = box.GetPrev();
  auto const pBelow = box.GetNext();
  boxes_.Delete(&box);
  auto const rc = box.rect();
  box.Removed();
  box.Release();
  DidRemoveBox(pAbove, pBelow, rc);

  if (!outer()) {
    return;
  }

  if (boxes_.IsEmpty()) {
    outer()->RemoveBox(*this);
    return;
  }

  if (boxes_.GetFirst() != boxes_.GetLast()) {
    return;
  }

  auto& first_box = *boxes_.GetFirst();
  boxes_.Delete(&first_box);
  auto const outer = this->outer();
  Removed();
  outer->Replace(first_box, *this);
  first_box.Release();
  return;
}

void EditPane::LayoutBox::Replace(Box& new_box, Box& old_box) {
  ASSERT(!is_removed());
  boxes_.InsertBefore(&new_box, &old_box);
  new_box.set_outer(*this);
  new_box.AddRef();
  boxes_.Delete(&old_box);
  old_box.Release();
}

void EditPane::LayoutBox::UpdateSplitters() {
  if (is_removed() || !edit_pane_->is_shown())
    return;
  auto& gfx = edit_pane_->frame().gfx();
  gfx::Graphics::DrawingScope drawing_scope(gfx);
  DrawSplitters(gfx);
}

// HitTestResult
EditPane::Window* EditPane::HitTestResult::window() const {
  return static_cast<LeafBox*>(box)->GetWindow();
}

// LeafBox
EditPane::LeafBox::~LeafBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  ASSERT(!m_pWindow);

  if (m_hwndVScrollBar) {
    ::SetWindowLongPtr(m_hwndVScrollBar, GWLP_USERDATA, 0);
    ::DestroyWindow(m_hwndVScrollBar);
    m_hwndVScrollBar = nullptr;
  }
}

void EditPane::LeafBox::CloseAllBut(Window* window) {
  if (GetWindow() != window) {
    GetWindow()->Destroy();
  }
}

void EditPane::LeafBox::Destroy() {
    GetWindow()->Destroy();
}

void EditPane::LeafBox::DetachWindow() {
  m_pWindow = nullptr;
  m_hwndVScrollBar = nullptr;
}

void EditPane::LeafBox::EnsureInHorizontalLayoutBox() {
  if (!outer()->IsVerticalLayoutBox()) {
    return;
  }

  auto& layout_box = *new HorizontalLayoutBox(edit_pane_, outer());
  ScopedRefCount_<LeafBox> protect(*this);
  outer()->Replace(layout_box, *this);
  layout_box.Realize(edit_pane_, rect());
  layout_box.Add(*this);
  set_outer(layout_box);
}

void EditPane::LeafBox::EnsureInVerticalLayoutBox() {
  if (outer()->IsVerticalLayoutBox()) {
    return;
  }

  auto& layout_box = *new VerticalLayoutBox(edit_pane_, outer());
  ScopedRefCount_<LeafBox> protect(*this);
  outer()->Replace(layout_box, *this);
  layout_box.Realize(edit_pane_, rect());
  layout_box.Add(*this);
  set_outer(layout_box);
}

EditPane::LeafBox* EditPane::LeafBox::GetActiveLeafBox() const {
  return const_cast<LeafBox*>(this);
}

EditPane::LeafBox* EditPane::LeafBox::GetFirstLeafBox() const {
  return const_cast<LeafBox*>(this);
}

EditPane::LeafBox* EditPane::LeafBox::FindLeafBoxFromWidget(
    const Widget& window) const {
  return window == m_pWindow ? const_cast<LeafBox*>(this) : nullptr;
}

EditPane::HitTestResult EditPane::LeafBox::HitTest(Point pt) const {
  if (!::PtInRect(&rect(), pt))
    return HitTestResult();

  auto const cxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
  if (pt.x < rect().right - cxVScroll)
    return HitTestResult(HitTestResult::Window, *this);

  if (!HasSibling() && pt.y < rect().top + k_cySplitterBig)
    return HitTestResult(HitTestResult::VSplitterBig, *this);

  return HitTestResult();
}

void EditPane::LeafBox::Realize(EditPane* edit_pane, const gfx::Rect& rect) {
  Box::Realize(edit_pane, rect);

  auto const splitter_height = HasSibling() ? 0 : k_cySplitterBig;
  auto const scroll_bar_width = ::GetSystemMetrics(SM_CXVSCROLL);
  Rect scroll_bar_rect(rect.right - scroll_bar_width,
                       rect.top + splitter_height,
                       rect.right, rect.bottom);

  if (m_pWindow->is<TextEditWindow>()) {
    m_hwndVScrollBar = ::CreateWindowExW(
          0,
          L"SCROLLBAR",
          nullptr, // title
          WS_CHILD | WS_VISIBLE | SBS_VERT,
          0, 0, 0, 0,
          edit_pane->AssociatedHwnd(),
          nullptr, // menu
          g_hInstance,
          nullptr);

    ::SetWindowLongPtr(m_hwndVScrollBar, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(m_pWindow));
  }

  Rect window_rect(rect.left, rect.top, scroll_bar_rect.left, rect.bottom);
  m_pWindow->Realize(window_rect);
  if (m_hwndVScrollBar)
    m_pWindow->as<TextEditWindow>()->SetScrollBar(m_hwndVScrollBar, SB_VERT);
  m_pWindow->Show();
  // Resize scrollbar
  SetRect(rect);
}

void EditPane::LeafBox::Redraw() const {
  m_pWindow->Redraw();
}

void EditPane::LeafBox::ReplaceWindow(Window* window) {
  DCHECK(!window->parent_node());
  DCHECK(!window->is_realized());
  auto const previous_window = m_pWindow;
  m_pWindow = window;
  m_hwndVScrollBar = nullptr;
  edit_pane_->AppendChild(*window);
  Realize(edit_pane_, rect());
  previous_window->Destroy();
}

void EditPane::LeafBox::SetRect(const gfx::Rect& rect) {
  Box::SetRect(rect);

  #if DEBUG_SPLIT
    DEBUG_PRINTF("%p %p %d+%d-%d+%d\n",
        this,
        m_pWindow,
        rect.left, rect.top, rect.right, rect.bottom);
  #endif

  auto const splitter_height = HasSibling() ? 0 : k_cySplitterBig;
  auto const scroll_bar_width = m_hwndVScrollBar ?
      ::GetSystemMetrics(SM_CXVSCROLL) : 0;
  Rect scroll_bar_rect(rect.right - scroll_bar_width,
                       rect.top + splitter_height,
                       rect.right, rect.bottom);
  if (m_hwndVScrollBar) {
    ::SetWindowPos(
        m_hwndVScrollBar,
        nullptr,
        scroll_bar_rect.left, scroll_bar_rect.top,
        scroll_bar_rect.width(),scroll_bar_rect.height(),
        SWP_NOZORDER);
  }

  Rect window_rect(rect.left, rect.top, scroll_bar_rect.left, rect.bottom);
  m_pWindow->ResizeTo(window_rect);
}

EditPane::VerticalLayoutBox::VerticalLayoutBox(EditPane* edit_pane,
                                               LayoutBox* outer)
    : LayoutBox(edit_pane, outer) {}

EditPane::VerticalLayoutBox::~VerticalLayoutBox() {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
}

void EditPane::VerticalLayoutBox::DidRemoveBox(
    Box* const pAbove,
    Box* const pBelow,
    const gfx::Rect& rc) {
  if (pAbove) {
    // Extend pane above.
    RECT rect = pAbove->rect();
    rect.bottom = rc.bottom;
    pAbove->SetRect(rect);

  } else if (pBelow) {
    // Extend pane below.
    RECT rect = pBelow->rect();
    rect.top = rc.top;
    pBelow->SetRect(rect);
  }
}

void EditPane::VerticalLayoutBox::DrawSplitters(const gfx::Graphics& gfx) {
  auto rc = rect();

  if (boxes_.GetFirst() == boxes_.GetLast()) {
    auto const cxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
    rc.left = rc.right - cxVScroll;
    rc.bottom = rc.top + k_cySplitterBig;
    DrawSplitter(gfx, &rc, BF_RECT);
    boxes_.GetFirst()->DrawSplitters(gfx);
    return;
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const box = it.Get();
    box->DrawSplitters(gfx);
    if (auto const above_box = box->GetPrev()) {
      rc.top = above_box->rect().bottom;
      rc.bottom = box->rect().top;
      DrawSplitter(gfx, &rc, BF_TOP | BF_BOTTOM);
    }
  }
}

EditPane::HitTestResult EditPane::VerticalLayoutBox::HitTest(
    Point pt) const {
  if (!::PtInRect(&rect(), pt)) {
    return HitTestResult();
  }

  foreach (BoxList::Enum, it, boxes_) {
    auto const result = it->HitTest(pt);
    if (result.type != HitTestResult::None) {
      return result;
    }

    if (auto const above_box = it->GetPrev()) {
      RECT splitterRect;
      splitterRect.left = rect().left;
      splitterRect.right = rect().right;
      splitterRect.top = above_box->rect().bottom;
      splitterRect.bottom = it->rect().top;
      if (::PtInRect(&splitterRect, pt)) {
        return HitTestResult(HitTestResult::VSplitter, *it.Get());
      }
    }
  }

  return HitTestResult();
}

bool EditPane::VerticalLayoutBox::IsVerticalLayoutBox() const {
  return true;
}

void EditPane::VerticalLayoutBox::MoveSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  auto const pBelow = &below_box;
  auto const pBelowPrev = pBelow->GetPrev();
  auto const pAbove = pBelowPrev
      ? pBelowPrev
      : &Split(*pBelow, k_cySplitter);

  if (pt.y - pAbove->rect().top <= 0) {
    // Above box is too small.
  } else if (pBelow->rect().bottom - (pt.y + k_cySplitter) <= 0) {
    // Below box is too small.
  } else {
    pAbove->rect().bottom = pt.y;
    pBelow->rect().top = pt.y + k_cySplitter;
    pAbove->SetRect(pAbove->rect());
    pBelow->SetRect(pBelow->rect());
  }

  UpdateSplitters();
}

void EditPane::VerticalLayoutBox::Realize(
    EditPane* edit_pane,
    const gfx::Rect& rect) {
  LayoutBox::Realize(edit_pane, rect);

  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->Realize(edit_pane, rect);
    return;
  }

  auto const height = rect.bottom - rect.top;
  auto const content_height = height - k_cySplitter * (num_boxes - 1);
  auto const box_height = content_height / num_boxes;
  RECT elemRect(rect);
  foreach (BoxList::Enum, it, boxes_) {
    elemRect.bottom = rect.top + box_height;
    it->Realize(edit_pane, elemRect);
    elemRect.top = elemRect.bottom + k_cySplitter;
  }
}

void EditPane::VerticalLayoutBox::SetRect(const gfx::Rect& newRect) {
  RECT rcOld = rect();
  LayoutBox::SetRect(newRect);
  auto const num_boxes = boxes_.Count();
  if (!num_boxes) {
    return;
  }

  if (num_boxes == 1) {
    boxes_.GetFirst()->SetRect(newRect);
    return;
  }

  auto const cyNewPane = rect().bottom  - rect().top;
  auto const cyOldPane = rcOld.bottom - rcOld.top;

  if (!cyOldPane) {
    auto const cBoxes = boxes_.Count();
    if (!cBoxes) {
      return;
    }

    auto const cyNewWin = cyNewPane / cBoxes;
    auto yBox = rect().top;
    auto cySplitter = 0;
    auto pBox = static_cast<Box*>(nullptr);
    foreach (BoxList::Enum, oEnum, boxes_) {
      pBox = oEnum.Get();
      auto const prc = &pBox->rect();
      yBox += cySplitter;
      prc->top = yBox;
      yBox += cyNewWin;
      prc->bottom = yBox;
      cySplitter = k_cySplitter;
    }

    if (pBox) {
      pBox->rect().bottom = rect().bottom;
    }
  } else {
    ScopedRefCount_<LayoutBox> protect(*this);
    tryAgain:
      auto yBox = rect().top;
      auto cySplitter = 0;
      auto pBox = static_cast<Box*>(nullptr);
      foreach (BoxList::Enum, oEnum, boxes_) {
        pBox = oEnum.Get();
        auto const prc = &pBox->rect();
        auto const cyOldWin = prc->bottom - prc->top;
        auto const cyNewWin = cyNewPane * cyOldWin / cyOldPane;
        if (cyNewWin < k_cyMinBox) {
          pBox->Destroy();
          if (is_removed()) {
            return;
          }
          goto tryAgain;
        }
        yBox += cySplitter;
        prc->top = yBox;
        yBox += cyNewWin;
        prc->bottom = yBox;
        cySplitter = k_cySplitter;
      }

      if (!pBox) {
        return;
      }
      pBox->rect().bottom = rect().bottom;
  }

  foreach (BoxList::Enum, oEnum, boxes_) {
    auto const pBox = oEnum.Get();
    auto newRect = pBox->rect();
    newRect.left = rect().left;
    newRect.right = rect().right;
    pBox->SetRect(newRect);
  }
}

EditPane::LeafBox& EditPane::VerticalLayoutBox::Split(
    Box& below_box,
    int cyBox) {
  auto pBelow = below_box.GetActiveLeafBox();
  auto const below = pBelow->GetWindow()->as<TextEditWindow>();
  if (!below)
    return *pBelow;
  auto const prcBelow = &pBelow->rect();
  ASSERT(prcBelow->bottom - prcBelow->top > cyBox);
  DCHECK_EQ(pBelow->GetWindow()->container_widget(), edit_pane_);

  auto const pWindow = new TextEditWindow(
      below->GetBuffer(),
      below->GetStart());
  edit_pane_->AppendChild(*pWindow);

  auto const pSelection = below->GetSelection();

  pWindow->GetSelection()->SetRange(
      pSelection->GetStart(),
      pSelection->GetEnd());

  pWindow->GetSelection()->SetStartIsActive(
      pSelection->IsStartActive());

  auto const pAbove = new LeafBox(edit_pane_, this, pWindow);
  auto const prcAbove = &pAbove->rect();

  boxes_.InsertBefore(pAbove, pBelow);
  pAbove->AddRef();

  prcAbove->left = prcBelow->left;
  prcAbove->right = prcBelow->right;
  prcAbove->top = prcBelow->top;
  prcAbove->bottom = prcBelow->top + cyBox;

  pAbove->Realize(edit_pane_, *prcAbove);

  prcBelow->top = prcAbove->bottom + k_cySplitter;
  pBelow->SetRect(pBelow->rect());

  UpdateSplitters();

  return *pAbove;
}

void EditPane::VerticalLayoutBox::StopSplitter(
    const gfx::Point& pt,
    Box& below_box) {
  #if DEBUG_RESIZE
    DEBUG_PRINTF("%p\n", this);
  #endif
  if (!below_box.GetPrev()) {
    return;
  }

  ScopedRefCount_<LayoutBox> protect(*this);
  auto& above_box = *below_box.GetPrev();
  auto const cyMin = k_cyMinBox;
  if (pt.y - above_box.rect().top < cyMin) {
    below_box.rect().top = above_box.rect().top;
    above_box.Destroy();
    below_box.SetRect(below_box.rect());
    UpdateSplitters();
  } else if (below_box.rect().bottom - pt.y < k_cyMinBox) {
    above_box.rect().bottom = below_box.rect().bottom;
    below_box.Destroy();
    above_box.SetRect(above_box.rect());
    UpdateSplitters();
  }
}

//////////////////////////////////////////////////////////////////////
//
// EditPane::SplitterController
//
class EditPane::SplitterController {
  public: enum State {
    State_None,
    State_Drag,
    State_DragSingle,
  };

  private: const EditPane& owner_;
  private: Box* m_pBox;
  private: State m_eState;

  public: explicit SplitterController(const EditPane&);
  public: ~SplitterController();

  public: bool is_dragging() const { return m_eState != State_None; }
  public: void End(const gfx::Point&);
  public: void Move(const gfx::Point&);
  public: void Start(State, Box&);
  public: void Stop();

  DISALLOW_COPY_AND_ASSIGN(SplitterController);
};

EditPane::SplitterController::SplitterController(const EditPane& owner)
    : owner_(owner),
      m_eState(State_None),
      m_pBox(nullptr) {}

EditPane::SplitterController::~SplitterController() {
  ASSERT(!m_pBox);
}

void EditPane::SplitterController::End(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    m_pBox->outer()->StopSplitter(point, *m_pBox);
    Stop();
  }
}

void EditPane::SplitterController::Move(const gfx::Point& point) {
  if (m_eState == SplitterController::State_Drag ||
      m_eState == SplitterController::State_DragSingle) {
    m_pBox->outer()->MoveSplitter(point, *m_pBox);
  }
}

void EditPane::SplitterController::Start(State eState, Box& box) {
  ASSERT(!!box.outer());
  owner_.SetCapture();
  m_eState = eState;
  m_pBox = &box;
  box.AddRef();
}

void EditPane::SplitterController::Stop() {
  if (m_eState != State_None) {
    ASSERT(!!m_pBox);
    owner_.ReleaseCapture();
    m_eState = State_None;
    m_pBox->Release();
    m_pBox = nullptr;
  }
  ASSERT(!m_pBox);
}

EditPane::EditPane(Buffer* pBuffer, Posn lStart)
    : EditPane(new TextEditWindow(pBuffer, lStart)) {
}

EditPane::EditPane(Window* pWindow)
    : m_eState(State_NotRealized),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          root_box_(*new VerticalLayoutBox(this, nullptr))),
      ALLOW_THIS_IN_INITIALIZER_LIST(
          splitter_controller_(new SplitterController(*this))) {
  AppendChild(*pWindow);
  ScopedRefCount_<LeafBox> box(*new LeafBox(this, root_box_, pWindow));
  root_box_->Add(*box);
}

EditPane::~EditPane() {
  root_box_->Removed();
}

Frame& EditPane::frame() const {
  ASSERT(GetFrame());
  return *GetFrame();
}

void EditPane::Activate() {
  Pane::Activate();
  auto const window = GetActiveWindow();
  if (!window)
    return;
  window->SetFocus();
}

void EditPane::CloseAllBut(Window* window) {
  root_box_->CloseAllBut(window);
}

void EditPane::DidRealize() {
  m_eState = State_Realized;
  root_box_->Realize(this, rect());
}

void EditPane::DidRealizeChildWidget(const Widget& window) {
  auto const box = root_box_->FindLeafBoxFromWidget(window);
  if (!box)
    return;

  auto const next_leaf_box = box->GetNext() ?
      box->GetNext()->GetFirstLeafBox() : nullptr;
   auto const next_window = next_leaf_box ? next_leaf_box->GetWindow() :
      nullptr;
  if (next_window)
    InsertBefore(*box->GetWindow(), next_window);
  else
    AppendChild(*box->GetWindow());
}

void EditPane::DidResize() {
  #if DEBUG_RESIZE
    DEBUG_WIDGET_PRINTF(DEBUG_RECT_FORMAT "\n", DEBUG_RECT_ARG(rect()));
  #endif
  root_box_->SetRect(rect());
  if (is_shown()) {
    gfx::Graphics::DrawingScope drawing_scope(frame().gfx());
    root_box_->DrawSplitters(frame().gfx());
  }
}

void EditPane::DidSetFocus() {
  if (auto const widget = GetActiveWindow())
    widget->SetFocus();
}

// Returns the last active Box.
EditPane::LeafBox* EditPane::GetActiveLeafBox() const {
  return root_box_->GetActiveLeafBox();
}

// Returns the last active Box.
EditPane::Window* EditPane::GetActiveWindow() const {
  auto const pBox = GetActiveLeafBox();
  return pBox ? pBox->GetWindow() : nullptr;
}

Buffer* EditPane::GetBuffer() const {
  auto const window = GetActiveWindow();
  if (!window)
    return nullptr;
  if (auto const text_edit_window = window->as<TextEditWindow>())
    return text_edit_window->GetBuffer();
  return nullptr;
}

HCURSOR EditPane::GetCursorAt(const gfx::Point& point) const {
  auto const result = root_box_->HitTest(point);
  switch (result.type) {
    case HitTestResult::HSplitter:
    case HitTestResult::HSplitterBig: {
      DEFINE_STATIC_LOCAL(StockCursor, hsplit_cursor, (IDC_HSPLIT));
      return hsplit_cursor;
    }

    case HitTestResult::VSplitter:
    case HitTestResult::VSplitterBig: {
      DEFINE_STATIC_LOCAL(StockCursor, vsplit_cursor, (IDC_VSPLIT));
      return vsplit_cursor;
    }

    case HitTestResult::None:
      return nullptr;

    default: {
      DEFINE_STATIC_LOCAL(StockCursor, arrow_cursor, (IDC_ARROW));
      return arrow_cursor;
    }
  }
}

EditPane::Window* EditPane::GetFirstWindow() const {
  auto const window = first_child()->as<Window>();
  if (window)
    return window;
  CAN_NOT_HAPPEN();
}

EditPane::Window* EditPane::GetLastWindow() const {
  auto const window = last_child()->as<Window>();
  if (window)
    return window;
  CAN_NOT_HAPPEN();
}

int EditPane::GetTitle(char16* out_wszTitle, int cchTitle) {
  const auto& title = GetActiveWindow()->GetTitle(cchTitle);
  ::CopyMemory(out_wszTitle, title.data(), title.size() * sizeof(char16));
  out_wszTitle[title.size()] = 0;
  return static_cast<int>(title.size());
}

Command::KeyBindEntry* EditPane::MapKey(uint nKey) {
  return GetActiveWindow()->MapKey(nKey);
}

void EditPane::OnLeftButtonDown(uint, const gfx::Point& point) {
  auto const result = root_box_->HitTest(point);
  if (result.type == HitTestResult::HSplitter ||
      result.type == HitTestResult::VSplitter) {
    splitter_controller_->Start(SplitterController::State_Drag,
                                *result.box);
  } else if (result.type == HitTestResult::VSplitterBig) {
    splitter_controller_->Start(SplitterController::State_DragSingle,
                                *result.box);
  }
}

void EditPane::OnLeftButtonUp(uint, const gfx::Point& point) {
  splitter_controller_->End(point);
}

void EditPane::OnMouseMove(uint, const gfx::Point& point) {
  splitter_controller_->Move(point);
}

void EditPane::ReplaceActiveWindow(Window* window) {
  DCHECK(!window->is_realized());
  GetActiveLeafBox()->ReplaceWindow(window);
}

EditPane::Window* EditPane::SplitHorizontally() {
  auto const right_box = GetActiveLeafBox();
  ASSERT(!!right_box);
  auto const right_rect = right_box->rect();

  // Active Box is too small to split.
  auto const cxBox = right_rect.right - right_rect.left;
  if (cxBox < k_cxMinBox * 2 + k_cxSplitter) {
    return nullptr;
  }

  right_box->EnsureInHorizontalLayoutBox();
  right_box->outer()->Split(*right_box, cxBox / 2);
  right_box->GetWindow()->MakeSelectionVisible();
  return right_box->GetWindow();
}

EditPane::Window* EditPane::SplitVertically() {
  auto const below_box = GetActiveLeafBox();
  ASSERT(!!below_box);
  auto const belowRect = below_box->rect();

  // Active Box is too small to split.
  auto const cyBox = belowRect.bottom - belowRect.top;
  if (cyBox < k_cyMinBox * 2 + k_cySplitter) {
    return nullptr;
  }

  below_box->EnsureInVerticalLayoutBox();
  below_box->outer()->Split(*below_box, cyBox / 2);
  below_box->GetWindow()->MakeSelectionVisible();
  return below_box->GetWindow();
}

void EditPane::UpdateStatusBar() {
  GetActiveWindow()->UpdateStatusBar();
}

void EditPane::WillDestroyWidget() {
  root_box_->Destroy();
}

void EditPane::WillDestroyChildWidget(const Widget& child) {
  Pane::WillDestroyChildWidget(child);
  auto const box = root_box_->FindLeafBoxFromWidget(child);
  if (!box) {
    // RepalceActiveWindow() removes window from box then destroys window.
    return;
  }

  #if DEBUG_RESIZE
    DEBUG_WIDGET_PRINTF("box=%p\n", box);
  #endif
  box->DetachWindow();
  auto const outer = box->outer();
  outer->RemoveBox(*box);
  if (m_eState != State_Realized)
    return;
  if (root_box_->CountLeafBox())
    return;
  frame().WillDestroyPane(this);

  // There is no window in this pane. So, we delete this pane.
  m_eState = State_Destroyed;
  root_box_->Destroy();
}
