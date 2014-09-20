// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_strip.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "common/castable.h"
#include "evita/dom/public/tab_data.h"
#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/events/event.h"
#include "evita/ui/tooltip.h"
#include "evita/views/frame_list.h"
#include "evita/views/icon_cache.h"
#include "evita/views/tab_content.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/vi_Frame.h"

namespace views {
namespace {

static const float kArrowButtonWidth = 20.0f;
static const float kArrowButtonHeight = 20.0f;
static const float kIconWidth = 16.0f;
static const float kIconHeight = 16.0f;
static const float kLabelHeight = 16.0f;
static const float kMaxTabWidth = 200.0f;
static const float kMinTabWidth = 140.0f;

//////////////////////////////////////////////////////////////////////
//
// Element
//
class Element : public common::Castable {
  DECLARE_CASTABLE_CLASS(Element, Castable);

  public: enum class State {
    Normal,
    Selected,
  };

  private: gfx::RectF bounds_;
  private: bool dirty_;
  private: bool is_hover_;
  private: Element* parent_;
  private: State state_;

  // ctor
  protected: Element(Element* parent);
  public: virtual ~Element() = default;

  protected: gfx::ColorF bgcolor() const;
  public: float bottom() const { return bounds_.bottom; }
  public: const gfx::RectF& bounds() const { return bounds_; }
  public: void set_bounds(const gfx::RectF& new_bounds);
  protected: bool dirty() const { return dirty_; }
  public: float height() const { return bounds_.height(); }
  public: float left() const { return bounds_.left; }
  public: Element* parent() const { return parent_; }
  public: gfx::PointF origin() const { return bounds_.origin(); }
  public: float right() const { return bounds_.right; }
  public: State state() const { return state_; }
  public: float top() const { return bounds_.top; }
  public: float width() const { return bounds_.width(); }

  // [D]
  public: virtual void Draw(gfx::Canvas* canvas) const = 0;

  // [H]
  public: virtual Element* HitTest(const gfx::PointF& point) const;

  // [I]
  public: void Invalidate();
  public: bool IsDescendantOf(const Element* other) const;
  public: bool IsHover() const { return is_hover_; }
  public: bool IsSelected() const { return State::Selected == state_; }

  // [S]
  public: bool SetHover(bool new_hover);
  public: Element* SetParent(Element* p);
  public: State SetState(State e);

  // [U]
  protected: virtual void Update();
};

Element::Element(Element* parent)
    : dirty_(true),
      is_hover_(false),
      parent_(parent),
      state_(State::Normal) {
}

gfx::ColorF Element::bgcolor() const {
  if (IsSelected())
      return gfx::whiteColor();
  if (IsHover())
      return gfx::sysColor(COLOR_3DHILIGHT, 0.8);
  return gfx::sysColor(COLOR_3DFACE, 0.5);
}

void Element::set_bounds(const gfx::RectF& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
}

Element* Element::HitTest(const gfx::PointF&  point) const {
  return bounds_.Contains(point) ? const_cast<Element*>(this) : nullptr;
}

void Element::Invalidate() {
  dirty_ = true;
}

bool Element::IsDescendantOf(const Element* other) const {
  for (auto runner = parent_; runner; runner = runner->parent_) {
    if (runner == other)
      return true;
  }
  return false;
}

bool Element::SetHover(bool f) {
  return is_hover_ = f;
}

Element* Element::SetParent(Element* p) {
  return parent_ = p;
}

Element::State Element::SetState(State e) {
  state_ = e;
  Update();
  return state_;
}

void Element::Update() {
}

//////////////////////////////////////////////////////////////////////
//
// TabStrip Design Parameters
//
enum TabStripDesignParams {
  k_cxMargin = 0,
  k_cxEdge = 2,
  k_cxBorder = 3,
  k_cxPad = 3,
  k_cyBorder = 5,
  k_cyIcon = 16,
};

//////////////////////////////////////////////////////////////////////
//
// CloseBox
//
class CloseBox final : public Element {
  DECLARE_CASTABLE_CLASS(CloseBox, Element);

  public: CloseBox(Element* parent);
  public: virtual ~CloseBox() = default;

  private: gfx::ColorF color() const;

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(CloseBox);
};

CloseBox::CloseBox(Element* parent) : Element(parent) {
}

gfx::ColorF CloseBox::color() const {
  return IsHover() ? gfx::ColorF(1, 0, 0, 0.5f) : gfx::ColorF(0, 0, 0, 0.5f);
}

void CloseBox::Draw(gfx::Canvas* canvas) const {
  auto const bounds = this->bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  gfx::Brush brush(canvas, color());
  canvas->DrawLine(brush, bounds.origin(), bounds.bottom_right(), 2.0f);
  canvas->DrawLine(brush, bounds.top_right(), bounds.bottom_left(), 2.0f);
}

//////////////////////////////////////////////////////////////////////
//
// Tab
//  Represents tab tab.
//
class Tab final : public Element, public ui::Tooltip::ToolDelegate {
  DECLARE_CASTABLE_CLASS(Tab, Element);

  private: CloseBox close_box_;
  public: int image_index_;
  private: gfx::RectF icon_bounds_;
  private: gfx::RectF text_bounds_;
  public: base::string16 label_text_;
  public: domapi::TabData::State state_;
  private: TabContent* const tab_content_;
  private: int tab_index_;
  private: TabStripDelegate* tab_strip_delegate_;

  public: Tab(TabStripDelegate* tab_strip_delegate, TabContent* tab_content);
  public: virtual ~Tab() = default;

  public: int tab_index() const { return tab_index_; }
  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }
  public: TabContent* tab_content() const { return tab_content_; }

  private: void DrawContent(gfx::Canvas* canvas) const;
  private: void DrawIcon(gfx::Canvas* canvas) const;
  public: bool HasCloseBox() const;
  public: bool SetTabData(const domapi::TabData& tab_data);
  public: void UpdateLayout();

  // Element
  public: virtual void Draw(gfx::Canvas* canvas) const override;
  public: virtual Element* HitTest(const gfx::PointF& point) const override;
  private: void Update() override;

  // ui::Tooltip::ToolDelegate
  private: base::string16 GetTooltipText() override;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

Tab::Tab(TabStripDelegate* tab_strip_delegate, TabContent* tab_content)
    : Element(nullptr),
      close_box_(this),
      image_index_(-1),
      state_(domapi::TabData::State::Normal),
      tab_content_(tab_content),
      tab_index_(0),
      tab_strip_delegate_(tab_strip_delegate) {
  auto const tab_data = tab_content->GetTabData();
  if (!tab_data) {
    label_text_ = L"?";
    return;
  }
  SetTabData(*tab_data);
}

void Tab::DrawContent(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  DrawIcon(canvas);
  gfx::Brush text_brush(canvas, gfx::sysColor(COLOR_BTNTEXT));
  canvas->DrawText(*canvas->work<gfx::TextFormat>(), text_brush, text_bounds_,
                   label_text_);
}

void Tab::DrawIcon(gfx::Canvas* canvas) const {
  if (image_index_ < 0)
    return;
  auto const hImageList = IconCache::instance()->image_list();
  // Note: ILD_TRANSPARENT doesn't effect.
  // Note: ILD_DPISCALE makes background black.
  auto const hIcon = ::ImageList_GetIcon(hImageList, image_index_, 0);
  if (!hIcon)
    return;
  gfx::Bitmap bitmap(canvas, hIcon);
  (*canvas)->DrawBitmap(bitmap, icon_bounds_);
  ::DestroyIcon(hIcon);
}

bool Tab::HasCloseBox() const {
  return IsSelected() || IsHover();
}

bool Tab::SetTabData(const domapi::TabData& tab_data) {
  struct Local {
    static int GetIconIndex(const domapi::TabData& tab_data) {
      if (tab_data.icon != -2)
        return tab_data.icon;
      return IconCache::instance()->GetIconForFileName(tab_data.title);
    }
  };

  auto changed = false;

  auto const new_image_index = Local::GetIconIndex(tab_data);
  if (image_index_ != new_image_index) {
    image_index_ = std::max(new_image_index, 0);
    changed = true;
  }

  if (state_ != tab_data.state) {
    state_ = tab_data.state;
    changed = true;
  }

  if (label_text_ != tab_data.title) {
    label_text_ = tab_data.title;
    changed = true;
  }
  return changed;
}

void Tab::UpdateLayout() {
  auto const bounds = this->bounds() - gfx::SizeF(6, 6);
  close_box_.set_bounds(gfx::RectF(bounds.top_right() + gfx::SizeF(-9, 5),
                                   gfx::SizeF(8, 8)));
  icon_bounds_ = gfx::RectF(bounds.origin(), gfx::SizeF(16, 16));
  text_bounds_ = gfx::RectF(icon_bounds_.top_right() + gfx::SizeF(4, 0),
                            gfx::PointF(close_box_.bounds().left - 2,
                                        icon_bounds_.bottom));
}

// Element
void Tab::Draw(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  {
    gfx::Brush fillBrush(canvas, bgcolor());
    canvas->FillRectangle(fillBrush, bounds());
    gfx::Brush strokeBrush(canvas, gfx::ColorF(0, 0, 0, 0.5));
    canvas->DrawRectangle(strokeBrush, gfx::RectF(
        bounds().origin(), bounds().size() + gfx::SizeF(1, 0)));
  }

  DrawContent(canvas);
  if (HasCloseBox())
    close_box_.Draw(canvas);
  if (state_ == domapi::TabData::State::Normal)
    return;
  auto const marker_color = state_ == domapi::TabData::State::Modified ?
    gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255) :
    gfx::ColorF(56.0f / 255, 219.0f / 255, 74.0f / 255);
  auto const marker_height = 4.0f;
  auto const marker_width = 4.0f;
  DCHECK_GT(width(), marker_width);
  canvas->FillRectangle(
      gfx::Brush(canvas, marker_color),
      gfx::RectF(gfx::PointF(right() - marker_width, top()),
                 gfx::SizeF(marker_width, marker_height)));
}

Element* Tab::HitTest(const gfx::PointF& point) const {
  if (HasCloseBox()) {
    if (auto const hit = close_box_.HitTest(point))
      return hit;
  }
  return Element::HitTest(point);
}

void Tab::Update() {
  UpdateLayout();
}

// ui::Tooltip::ToolDelegate
base::string16 Tab::GetTooltipText() {
  return tab_strip_delegate_->GetTooltipTextForTab(tab_index_);
}

//////////////////////////////////////////////////////////////////////
//
// ArrowButton
//
enum class Direction {
 Down,
 Left,
 Right,
 Up,
};

class ArrowButton final : public Element {
  DECLARE_CASTABLE_CLASS(ArrowButton, Element);

  private: Direction direction_;

  public: ArrowButton(Direction);
  public: virtual ~ArrowButton() = default;

  public: Direction direction() const { return direction_; }

  private: gfx::ColorF ComputeBgColor() const;

  private: void DrawArrow(gfx::Canvas* canvas) const;

  // Element
  private: virtual void Draw(gfx::Canvas* canvas) const override;

  DISALLOW_COPY_AND_ASSIGN(ArrowButton);
};

ArrowButton::ArrowButton(Direction direction)
    : Element(nullptr), direction_(direction) {
}

gfx::ColorF ArrowButton::ComputeBgColor() const {
  if (IsHover())
    return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.3f);
  return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.0f);
}

void ArrowButton::DrawArrow(gfx::Canvas* canvas) const {
  float factors[4] = {0.0f};
  switch (direction_) {
    case Direction::Down:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = -1.0f;
      break;
    case Direction::Left:
      factors[0] = 1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Right:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = -1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Up:
      factors[0] = -1.0f;
      factors[1] = 1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
     default:
       NOTREACHED();
   }

  auto const center_x = left() + width() / 2;
  auto const center_y = top() + height() / 2;
  auto const wing_size = width() / 4;
  auto const pen_width = 2.0f;

  auto const alpha = IsHover() ? 1.0f : 0.3f;
  gfx::Brush arrow_brush(canvas, gfx::ColorF(0.0f, 0.0f, 0.0f, alpha));

  canvas->DrawLine(arrow_brush,
                   gfx::PointF(center_x + factors[0] * wing_size,
                               center_y + factors[1] * wing_size),
                   gfx::PointF(center_x, center_y), pen_width);
  canvas->DrawLine(arrow_brush,
                   gfx::PointF(center_x + factors[2] * wing_size,
                               center_y + factors[3] * wing_size),
                   gfx::PointF(center_x, center_y), pen_width);
}

// Element
void ArrowButton::Draw(gfx::Canvas* canvas) const {
  if (bounds().empty())
    return;

  gfx::Brush fillBrush(canvas, ComputeBgColor());
  canvas->FillRectangle(fillBrush, bounds());
  DrawArrow(canvas);
}

// Cursor for Tab Drag
HCURSOR s_hDragTabCursor;

// Load cursor for Tab Drag and Drop.
void LoadDragTabCursor() {
  if (s_hDragTabCursor)
    return;

  s_hDragTabCursor = ::LoadCursor(nullptr, IDC_ARROW);

  auto const hDll = ::LoadLibraryEx(L"ieframe.dll", nullptr,
                                    LOAD_LIBRARY_AS_DATAFILE);
  if (!hDll)
    return;

  if (auto const hCursor = ::LoadCursor(hDll, MAKEINTRESOURCE(643))) {
    if (auto const hCursor2 = CopyCursor(hCursor))
      s_hDragTabCursor = hCursor2;
  }

  ::FreeLibrary(hDll);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Impl class
//
class TabStrip::Impl final {
  private: enum class Drag {
    None,
    Tab,
    Start,
  };

  private: typedef std::vector<Tab*> Tabs;

  private: gfx::RectF bounds_;
  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: TabStripDelegate* delegate_;
  private: Tab* dragging_tab_;
  private: Drag drag_state_;
  private: POINT drag_start_point_;
  private: Element* hover_element_;
  private: Tab* insertion_marker_;
  private: ArrowButton list_button_;
  private: ArrowButton scroll_left_button_;
  private: ArrowButton scroll_right_button_;
  private: Tab* selected_tab_;
  private: bool should_selected_tab_visible_;
  private: Tabs tabs_;
  private: HMENU tab_list_menu_;
  private: gfx::RectF tabs_bounds_;
  private: float tabs_origin_;
  private: ui::Tooltip tooltip_;
  private: TabStrip* widget_;

  public: Impl(TabStrip* widget, TabStripDelegate* delegate);
  public: ~Impl();

  public: size_t number_of_tabs() const { return tabs_.size(); }
  public: size_t selected_index() const;

  // [C]
  private: bool ChangeFont();

  // [D]
  public: void DidBeginAnimationFrame(base::Time time);
  private: void DidSelectTab();
  public: void DidRealize();
  public: void DeleteTab(size_t tab_index);
  private: void Draw(gfx::Canvas* canvas) const;
  private: void DropTab(Tab* tab, const POINT& point);

  // [G]
  public: TabContent* GetTab(size_t tab_index) const;

  // [H]
  private: void HandleTabListMenu(POINT point);
  private: Element* HitTest(const gfx::PointF& point) const;

  // [I]
  // Insert a new tab before a tab at |tab_index|.
  public: void InsertTab(size_t tab_index, TabContent* tab_content);

  // [O]
  public: void OnLButtonDown(POINT pt);
  public: void OnLButtonUp(POINT pt);
  public: void OnMouseMove(POINT pt);
  public: LRESULT OnNotify(NMHDR* nmhder);

  // [R]
  public: void Redraw();
  private: void RenumberTabIndex();

  // [S]
  public: int SelectTab(size_t tab_index);
  private: int SelectTab(Tab* tab);
  public: void SetBounds(const gfx::RectF& bounds);
  public: void SetTabData(size_t tab_index, const domapi::TabData& tab_data);
  private: void StopDrag();

  // [U]
  private: void UpdateBoundsForAllTabs(float tab_width);
  public: void UpdateHover(Element* pHover);
  private: void UpdateLayout();

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

TabStrip::Impl::Impl(TabStrip* widget, TabStripDelegate* delegate)
    : delegate_(delegate),
      dragging_tab_(nullptr),
      drag_state_(Drag::None),
      hover_element_(nullptr),
      insertion_marker_(nullptr),
      list_button_(Direction::Down),
      scroll_left_button_(Direction::Left),
      scroll_right_button_(Direction::Right),
      selected_tab_(nullptr),
      should_selected_tab_visible_(false),
      tab_list_menu_(nullptr),
      tabs_origin_(0),
      widget_(widget){
}

TabStrip::Impl::~Impl() {
  if (auto const text_format = canvas_->work<gfx::TextFormat>())
      delete text_format;

  if (tab_list_menu_)
    ::DestroyMenu(tab_list_menu_);
}

size_t TabStrip::Impl::selected_index() const {
  return selected_tab_ ? selected_tab_->tab_index() : static_cast<size_t>(-1);
}

bool TabStrip::Impl::ChangeFont() {
  LOGFONT lf;
  if (!::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
    return false;
  lf.lfHeight = -10;
  if (auto const old_format = canvas_->work<gfx::TextFormat*>())
    delete old_format;
  auto const text_format = new gfx::TextFormat(lf);
  {
    common::ComPtr<IDWriteInlineObject> inline_object;
    COM_VERIFY(gfx::FactorySet::instance()->dwrite().
        CreateEllipsisTrimmingSign(*text_format, &inline_object));
    DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    (*text_format)->SetTrimming(&trimming, inline_object);
  }
  (*text_format)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  canvas_->set_work(text_format);
  return true;
}

void TabStrip::Impl::DeleteTab(size_t tab_index) {
  if (tab_index >= tabs_.size())
    return;
  auto selection_changed = false;
  auto const tab = tabs_[tab_index];
  if (tabs_.size() == 1) {
    tabs_.clear();
    selected_tab_ = nullptr;
    hover_element_ = nullptr;
  } else {
    if (hover_element_ && (hover_element_ == tab ||
                           hover_element_->IsDescendantOf(tab))) {
      hover_element_ = nullptr;
    }
    if (tab == selected_tab_) {
      selection_changed = true;
      selected_tab_ = nullptr;
    }
    tabs_.erase(tabs_.begin() + static_cast<ptrdiff_t>(tab_index));
    RenumberTabIndex();
  }
  tooltip_.DeleteTool(tab);
  delete tab;

  Redraw();
  if (!selection_changed)
    return;
  DidSelectTab();
}

void TabStrip::Impl::DidBeginAnimationFrame(base::Time) {
  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  canvas_->AddDirtyRect(canvas_->bounds());
  Draw(canvas_.get());
  widget_->RequestAnimationFrame();
}

void TabStrip::Impl::DidSelectTab() {
  delegate_->DidSelectTab(selected_tab_ ? selected_tab_->tab_index() : -1);
}

void TabStrip::Impl::DidRealize() {
  canvas_.reset(widget_->layer()->CreateCanvas());
  ChangeFont();
  tooltip_.Realize(widget_->AssociatedHwnd());
}

void TabStrip::Impl::Draw(gfx::Canvas* canvas) const {
  struct Local {
    static void DrawInsertMarker(gfx::Canvas* canvas, Tab* insertion_marker) {
      if (!insertion_marker)
        return;
      auto bounds = insertion_marker->bounds();
      bounds.top += 5;
      bounds.bottom -= 7;
      gfx::Brush brush(canvas, gfx::ColorF::Black);
      for (int w = 1; w <= 7; w += 2) {
        canvas->FillRectangle(brush, gfx::RectF(
            gfx::PointF(bounds.left, bounds.top), gfx::SizeF(w, 1)));
        canvas->FillRectangle(brush, gfx::RectF(
            gfx::PointF(bounds.left, bounds.bottom), gfx::SizeF(w, 1)));
        bounds.top -= 1;
        bounds.left -= 1;
        bounds.bottom += 1;
      }
    }
  };

 canvas->Clear(gfx::ColorF(0, 0, 1, 0.0));
 if (tabs_.empty())
   return;

  static_cast<const Element&>(scroll_left_button_).Draw(canvas);
  static_cast<const Element&>(list_button_).Draw(canvas);
  static_cast<const Element&>(scroll_right_button_).Draw(canvas);

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_.get(), tabs_bounds_);
  for (auto const tab : tabs_) {
    if (tab->right() < 0)
      continue;
    if (tab->left() > bounds_.right)
      break;
    tab->Draw(canvas);
  }

  Local::DrawInsertMarker(canvas, insertion_marker_);
}

// Send TabDragMsg to window which can handle it.
void TabStrip::Impl::DropTab(Tab* tab, const POINT& window_point) {
  auto screen_point = widget_->MapToDesktopPoint(window_point);

  for (auto hwnd = ::WindowFromPoint(screen_point); hwnd;
       hwnd = ::GetParent(hwnd)) {
    if (auto const frame = FrameList::instance()->FindFrameByHwnd(hwnd)) {
      static_cast<TabStripDelegate*>(frame)->OnDropTab(tab->tab_content());
      return;
    }
  }

  delegate_->DidThrowTab(tab->tab_content());
}

TabContent* TabStrip::Impl::GetTab(size_t tab_index) const {
  if (tab_index >= tabs_.size())
    return nullptr;
  return tabs_[tab_index]->tab_content();
}

void TabStrip::Impl::HandleTabListMenu(POINT) {
  gfx::Point local_menu_origin(
      static_cast<int>(list_button_.origin().x),
      static_cast<int>(list_button_.origin().y));
  auto const menu_origin = widget_->MapToDesktopPoint(local_menu_origin);
  if (!tab_list_menu_)
    tab_list_menu_ = ::CreatePopupMenu();

  // Make Tab List Menu empty
  while (::GetMenuItemCount(tab_list_menu_) > 0)
    ::DeleteMenu(tab_list_menu_, 0, MF_BYPOSITION);

  // Add Tab name to menu.
  auto last_tab = static_cast<Tab*>(nullptr);
  for (auto const tab : tabs_) {
    auto const flags = tab->IsSelected() ? MF_STRING | MF_CHECKED :
                                           MF_STRING;
    if (last_tab && last_tab->right() < 0 != tab->right() < 0)
      ::AppendMenu(tab_list_menu_, MF_SEPARATOR, 0, nullptr);
    last_tab = tab;
    ::AppendMenu(tab_list_menu_, static_cast<DWORD>(flags),
                 static_cast<DWORD>(tab->tab_index()),
                 tab->label_text_.c_str());
  }

  ::TrackPopupMenuEx(tab_list_menu_, TPM_LEFTALIGN | TPM_TOPALIGN, 
      menu_origin.x(), menu_origin.y(), widget_->AssociatedHwnd(), nullptr);
}

Element* TabStrip::Impl::HitTest(const gfx::PointF& point) const {
  if (auto const hit_result = scroll_left_button_.HitTest(point))
    return hit_result;
  if (auto const hit_result = scroll_right_button_.HitTest(point))
    return hit_result;
  if (auto const hit_result = list_button_.HitTest(point))
    return hit_result;

  for (auto const tab : tabs_) {
    if (auto const hit_result = tab->HitTest(point))
      return hit_result;
  }

  return nullptr;
}

void TabStrip::Impl::InsertTab(size_t tab_index_in,
                                       TabContent* tab_content) {
  auto const tab_index = std::min(tab_index_in, tabs_.size());
  auto const new_tab = new Tab(delegate_, tab_content);
  tabs_.insert(tabs_.begin() + static_cast<ptrdiff_t>(tab_index), new_tab);
  RenumberTabIndex();
  tooltip_.AddTool(new_tab);
  Redraw();
}

void TabStrip::Impl::OnLButtonDown(POINT point) {
  auto const element = HitTest(point);
  if (!element)
    return;

  if (auto arrow = element->as<ArrowButton>()) {
    auto const scroll_width = 10.0f;
    switch (arrow->direction()) {
      case Direction::Down:
        HandleTabListMenu(point);
        break;
      case Direction::Left: {
        auto const new_tabs_origin = std::min(tabs_origin_ + scroll_width,
                                              0.0f);
        if (tabs_origin_ == new_tabs_origin)
          break;
        tabs_origin_ = new_tabs_origin;
        should_selected_tab_visible_ = false;
        Redraw();
        break;
      }
      case Direction::Right: {
        auto const min_tabs_origin = tabs_bounds_.width() -
            tabs_.size() * tabs_.front()->width();
        auto const new_tabs_origin = std::max(tabs_origin_ - scroll_width,
                                              min_tabs_origin);
        if (tabs_origin_ == new_tabs_origin)
          break;
        tabs_origin_ = new_tabs_origin;
        should_selected_tab_visible_ = false;
        Redraw();
        break;
      }
    }
    return;
  }

  auto const tab = element->as<Tab>();
  if (!tab) {
    // Not a tab.
    return;
  }

  if (!tab->IsSelected())
    delegate_->RequestSelectTab(tab->tab_index());

  // Note: We should start tab dragging, otherwise if mouse pointer is in
  // close box, |OnButtonUp| close the tab.
  LoadDragTabCursor();

  dragging_tab_ = tab;
  drag_state_ = Drag::Start;
  drag_start_point_ = point;

  widget_->SetCapture();
}

void TabStrip::Impl::OnLButtonUp(POINT point) {
  if (!dragging_tab_) {
    auto const element = HitTest(point);
    if (!element)
      return;

    if (element->is<CloseBox>()) {
      if (auto const tab = element->parent()->as<Tab>())
        delegate_->RequestCloseTab(tab->tab_index());
      return;
    }

    return;
  }

  auto const dragging_tab = dragging_tab_;
  auto const insertion_marker = insertion_marker_;
  StopDrag();

  if (!insertion_marker) {
    DropTab(dragging_tab, point);
    return;
  }

  if (dragging_tab != insertion_marker) {
    tabs_.erase(tabs_.begin() + dragging_tab->tab_index());
    tabs_.insert(tabs_.begin() + insertion_marker->tab_index(), dragging_tab);
    RenumberTabIndex();
    UpdateLayout();
  }

  // Hide insertion position mark
  //::InvalidateRect(hwnd_, nullptr, false);
}

void TabStrip::Impl::OnMouseMove(POINT point) {
  auto const hover_element = HitTest(point);

  if (!dragging_tab_) {
    UpdateHover(hover_element);
    return;
  }

  if (::GetCapture() != widget_->AssociatedHwnd()) {
    // Someone takes capture. So, we stop dragging.
    StopDrag();
    return;
  }

  if (Drag::Start == drag_state_) {
    if (point.x - drag_start_point_.x >= -5 &&
        point.x - drag_start_point_.x <= 5) {
      return;
    }

    drag_state_ = Drag::Tab;
  }

  // Tab dragging
  auto const insertion_marker = hover_element ? hover_element->as<Tab>() :
                                                nullptr;
  ::SetCursor(s_hDragTabCursor);
  if (insertion_marker == insertion_marker_)
    return;

  if (insertion_marker)
    insertion_marker->Invalidate();

  if (insertion_marker_)
    insertion_marker_->Invalidate();

  insertion_marker_ = insertion_marker;
}

LRESULT TabStrip::Impl::OnNotify(NMHDR* nmhdr) {
  tooltip_.OnNotify(nmhdr);
  return 0;
}

void TabStrip::Impl::Redraw() {
  UpdateLayout();
}

void TabStrip::Impl::RenumberTabIndex() {
  auto tab_index = 0;
  for (auto tab : tabs_) {
    tab->set_tab_index(tab_index);
    ++tab_index;
  }
}

int TabStrip::Impl::SelectTab(size_t tab_index) {
  if (tab_index >= tabs_.size())
    return -1;
  return SelectTab(tabs_[tab_index]);
}

int TabStrip::Impl::SelectTab(Tab* const tab) {
  should_selected_tab_visible_ = true;
  if (selected_tab_ != tab) {
    if (selected_tab_) {
      selected_tab_->SetState(Element::State::Normal);
      selected_tab_->Invalidate();
    }

    selected_tab_ = tab;

    if (tab) {
      tab->SetState(Element::State::Selected);
      Redraw();
    }

    DidSelectTab();
  }

  return selected_tab_ ? selected_tab_->tab_index() : -1;
}

void TabStrip::Impl::SetBounds(const gfx::RectF& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  tabs_bounds_ = bounds_;
  canvas_->SetBounds(bounds_);
  Redraw();
}

void TabStrip::Impl::SetTabData(size_t tab_index,
                                        const domapi::TabData& tab_data) {
  if (tab_index >= tabs_.size())
    return;
  auto const tab = tabs_[tab_index];
  if (!tab->SetTabData(tab_data))
    return;
  tab->Invalidate();
}

void TabStrip::Impl::StopDrag() {
  drag_state_ = Drag::None;
  dragging_tab_ = nullptr;
  insertion_marker_ = nullptr;

  widget_->ReleaseCapture();
  ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));
}

void TabStrip::Impl::UpdateBoundsForAllTabs(float tab_width) {
  auto origin = gfx::PointF(tabs_origin_ + tabs_bounds_.left,
                            tabs_bounds_.top);
  auto const tab_size = gfx::SizeF(tab_width,
                                   tabs_bounds_.bottom - origin.y);
  for (auto const tab : tabs_){
    tab->set_bounds(gfx::RectF(origin, tab_size));
    tab->UpdateLayout();
    origin.x += tab_width;
    auto const visible_bounds = bounds_.Intersect(tab->bounds());
    // Note: We should pass bounds in HWND's coordinate rather than |TabStrip|
    // coordinate.
    auto const tool_bounds = visible_bounds.Offset(
        gfx::PointF(widget_->origin()));
    tooltip_.SetToolBounds(tab, gfx::ToEnclosingRect(tool_bounds));
  }
}

void TabStrip::Impl::UpdateHover(Element* hover_element) {
    if (hover_element_ == hover_element)
      return;

  if (hover_element_) {
    if (!hover_element || !hover_element->is<CloseBox>() ||
        hover_element->parent() != hover_element_) {
      if (hover_element_->is<CloseBox>()) {
        hover_element_->SetHover(false);
        hover_element_ = hover_element_->parent();
      }
      hover_element_->SetHover(false);
      hover_element_->Invalidate();
    }
  }

  hover_element_ = hover_element;
  if (hover_element_) {
    hover_element_->SetHover(true);
    hover_element_->Invalidate();
  }
}

void TabStrip::Impl::UpdateLayout() {
  if (tabs_.empty()) {
    tabs_origin_ = 0;
    return;
  }

  auto const tab_width = std::min(
      std::max(bounds_.width() / tabs_.size(), kMinTabWidth), kMaxTabWidth);

  if (tabs_.size() * tab_width < bounds_.width()) {
    scroll_left_button_.set_bounds(gfx::RectF());
    scroll_right_button_.set_bounds(gfx::RectF());
    list_button_.set_bounds(gfx::RectF());
    tabs_bounds_.left = bounds_.left;
    tabs_bounds_.right = bounds_.right;
    tabs_origin_ = 0;
  } else {
    auto const button_top = tabs_bounds_.top;
    auto const button_height = tabs_bounds_.height();
    scroll_left_button_.set_bounds(gfx::RectF(
        gfx::PointF(bounds_.left, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    list_button_.set_bounds(gfx::RectF(
        gfx::PointF(bounds_.right - kArrowButtonWidth, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    scroll_right_button_.set_bounds(gfx::RectF(
        gfx::PointF(list_button_.left() - kArrowButtonWidth, button_top),
        gfx::SizeF(kArrowButtonWidth, button_height)));
    tabs_bounds_.left = scroll_left_button_.right();
    tabs_bounds_.right = scroll_right_button_.left();
  }

  UpdateBoundsForAllTabs(tab_width);

  if (!selected_tab_ || !should_selected_tab_visible_ ||
      tabs_bounds_.Contains(selected_tab_->bounds())) {
    return;
  }

  // Make selected tab visible.
  if ((selected_tab_->tab_index() + 1) * tab_width < tabs_bounds_.width()) {
    tabs_origin_ = 0.0f;
  } else if ((tabs_.size() - selected_tab_->tab_index()) * tab_width <
             tabs_bounds_.width()) {
    tabs_origin_ = -(tabs_.size() * tab_width) + tabs_bounds_.width();
  } else if (selected_tab_->left() >= tabs_bounds_.left &&
             selected_tab_->left() < tabs_bounds_.right) {
    tabs_origin_ -= selected_tab_->right() - tabs_bounds_.right +
                    tab_width / 3;
  } else {
    tabs_origin_ += -selected_tab_->left() + tab_width / 3;
  }
  UpdateBoundsForAllTabs(tab_width);
}

//////////////////////////////////////////////////////////////////////
//
// TabStrip
//
TabStrip::TabStrip(TabStripDelegate* delegate)
    : impl_(new Impl(this, delegate)) {
}

TabStrip::~TabStrip() {
}

int TabStrip::number_of_tabs() const {
  return static_cast<int>(impl_->number_of_tabs());
}

int TabStrip::selected_index() const {
  return static_cast<int>(impl_->selected_index());
}

void TabStrip::DeleteTab(int tab_index) {
  impl_->DeleteTab(static_cast<size_t>(tab_index));
}

TabContent* TabStrip::GetTab(int tab_index) {
  return impl_->GetTab(static_cast<size_t>(tab_index));
}

void TabStrip::InsertTab(int new_tab_index, TabContent* tab_content) {
  impl_->InsertTab(static_cast<size_t>(new_tab_index), tab_content);
}

void TabStrip::SelectTab(int tab_index) {
  impl_->SelectTab(static_cast<size_t>(tab_index));
}

void TabStrip::SetTab(int tab_index, const domapi::TabData& tab_data) {
  impl_->SetTabData(static_cast<size_t>(tab_index), tab_data);
}

// ui::AnimationFrameHanndler
void TabStrip::DidBeginAnimationFrame(base::Time time) {
  impl_->DidBeginAnimationFrame(time);
}

// ui::Widget
void TabStrip::DidChangeBounds() {
  ui::AnimatableWindow::DidChangeBounds();
  if (layer())
    layer()->SetBounds(bounds());
  impl_->SetBounds(GetContentsBounds());
}

void TabStrip::DidRealize() {
  ui::AnimatableWindow::DidRealize();
  SetLayer(new ui::Layer());
  layer()->SetBounds(bounds());
  impl_->DidRealize();
}

// On Win8.1
//  SM_CYSIZE = 22
//  SM_CYCAPTION = 23
//  SM_CYEDGE = 2
//  SM_CYSIZEFRAME = 4
gfx::Size TabStrip::GetPreferredSize() const {
  return Size(300, 28);
}

void TabStrip::OnMouseExited(const ui::MouseEvent&) {
  impl_->UpdateHover(nullptr);
}

void TabStrip::OnMouseMoved(const ui::MouseEvent& event) {
  impl_->OnMouseMove(event.location());
}

void TabStrip::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  impl_->OnLButtonDown(event.location());
}

void TabStrip::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button())
    return;
  impl_->OnLButtonUp(event.location());
}

LRESULT TabStrip::OnNotify(NMHDR* nmhdr) {
  return impl_->OnNotify(nmhdr);
}

}  // namespace views
