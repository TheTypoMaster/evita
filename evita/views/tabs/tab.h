// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tabs_tab_h)
#define INCLUDE_evita_views_tabs_tab_h

#include <memory>

#include "evita/dom/public/tab_data.h"
#include "evita/gfx/rect_f.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/animation/animation_group_member.h"
#include "evita/ui/tooltip.h"
#include "evita/ui/widget.h"

namespace gfx {
class TextLayout;
}

namespace ui {
class AnimationFloat;
}

namespace views {

class Tab;
class TabContent;

//////////////////////////////////////////////////////////////////////
//
// TabController
//
class TabController {
  protected: TabController();
  protected: virtual ~TabController();

  public: virtual void AddTabAnimation(ui::AnimationGroupMember* member) = 0;
  public: virtual void DidChangeTabBounds(Tab* tab) = 0;
  public: virtual void DidDropTab(Tab* tab, const gfx::Point& screen_point) = 0;
  public: virtual void DidSelectTab(Tab* tab) = 0;
  public: virtual void MaybeStartDrag(Tab* tab,
                                      const gfx::Point& location) = 0;
  public: virtual void RemoveTabAnimation(ui::AnimationGroupMember* member) = 0;
  public: virtual void RequestCloseTab(Tab* tab) = 0;
  public: virtual void RequestSelectTab(Tab* tab) = 0;

  DISALLOW_COPY_AND_ASSIGN(TabController);
};

//////////////////////////////////////////////////////////////////////
//
// Tab
//  Represents a tab.
//
class Tab final : public ui::Widget,
                  private ui::AnimationGroupMember,
                  public ui::Tooltip::ToolDelegate {
  public: enum class Part {
    None,
    CloseMark,
    Label,
  };

  public: class HitTestResult final {
    private: Part part_;
    private: Tab* tab_;

    public: HitTestResult(Tab* tab, Part part);
    public: HitTestResult(const HitTestResult& other);
    public: HitTestResult();
    public: ~HitTestResult() = default;

    public: operator bool() const { return tab_ != nullptr; }

    public: HitTestResult& operator=(const HitTestResult& other);

    public: bool operator==(const HitTestResult& other) const;
    public: bool operator!=(const HitTestResult& other) const;

    public: Part part() const { return part_; }
    public: Tab* tab() const { return tab_; }
  };

  private: enum class State {
    Normal,
    Hovered,
    Selected,
  };

  private: float animated_alpha_;
  private: std::unique_ptr<ui::AnimationFloat> animation_alpha_;
  private: gfx::RectF close_mark_bounds_;
  private: State close_mark_state_;
  private: bool dirty_visual_;
  private: bool dirty_layout_;
  private: int image_index_;
  private: gfx::RectF icon_bounds_;
  private: gfx::RectF label_bounds_;
  private: base::string16 label_text_;
  private: State state_;
  private: TabContent* const tab_content_;
  private: TabController* const tab_controller_;
  private: domapi::TabData::State tab_data_state_;
  private: int tab_index_;
  private: gfx::TextFormat* text_format_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Tab(TabController* tab_controller, TabContent* tab_content,
              gfx::TextFormat* text_format);
  public: virtual ~Tab() override;

  public: bool is_selected() const { return state_ == State::Selected; }
  public: const base::string16& label_text() const { return label_text_; }
  public: TabContent* tab_content() const { return tab_content_; }
  public: int tab_index() const { return tab_index_; }

  public: void set_tab_index(int tab_index) { tab_index_ = tab_index; }

  private: static float ComputeAlpha(State state);
  private: gfx::ColorF ComputeBackgroundColor() const;
  private: void DrawCloseMark(gfx::Canvas* canvas) const;
  private: void DrawLabel(gfx::Canvas* canvas) const;
  private: void DrawIcon(gfx::Canvas* canvas) const;
  private: void DrawTabDataState(gfx::Canvas* canvas) const;
  public: int GetPreferredWidth() const;
  public: State GetState(Part part) const;
  public: HitTestResult HitTest(const gfx::PointF& point);
  public: void MarkDirty();
  public: void Select();
  private: void SetCloseMarkState(State new_state);
  private: void SetLabelState(State new_state);
  private: void SetState(Part part, State new_state);
  public: void SetTabData(const domapi::TabData& tab_data);
  public: void SetTextFormat(gfx::TextFormat* text_format_);
  public: void Unselect();
  private: void UpdateLayout();

  // ui::AnimationGroupMember
  private: virtual void Animate(base::Time time) override;

  // ui::Tooltip::ToolDelegate
  private: virtual base::string16 GetTooltipText() override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void OnDraw(gfx::Canvas* canvas) override;
  private: virtual void OnMouseEntered(const ui::MouseEvent& event) override;
  private: virtual void OnMouseExited(const ui::MouseEvent& event) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(Tab);
};

}   // views

#endif // !defined(INCLUDE_evita_views_tabs_tab_h)
