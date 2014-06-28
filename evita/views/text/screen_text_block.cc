// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/screen_text_block.h"

#include <unordered_set>

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/gfx/bitmap.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/views/switches.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

#define DEBUG_DRAW 0
// USE_OVERLAY controls how redraw marker rendered. If USE_OVERLAY is true,
// redraw marker is drawn as overlay rectangle, otherwise, redraw marker
// is rendered at left border.
#define USE_OVERLAY 1

namespace views {
namespace rendering {

namespace {

const auto kMarkerLeftMargin = 2.0f;
const auto kMarkerWidth = 4.0f;

void AddRect(std::vector<gfx::RectF>& rects, const gfx::RectF& rect) {
  if (rects.empty()) {
    rects.push_back(rect);
    return;
  }

  auto& last = rects.back();
  if (last.bottom == rect.top) {
    last.right = std::max(last.right, rect.right);
    last.bottom = rect.bottom;
    return;
  }

  if (last.top == rect.bottom) {
    last.right = std::max(last.right, rect.right);
    last.top = rect.top;
    return;
  }

  rects.push_back(rect);
}

inline gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

std::unordered_set<gfx::RectF> CalculateSelectionRects(
    const std::vector<TextLine*>& lines, const TextSelection& selection) {
  std::unordered_set<gfx::RectF> rects;
  if (selection.is_caret())
    return rects;
  if (selection.start() >= lines.back()->text_end())
    return rects;
  if (selection.end() <= lines.front()->text_start())
    return rects;
  for (auto line : lines) {
    if (selection.end() <= line->text_start())
        break;
    auto const rect = line->CalculateSelectionRect(selection);
    if (rect.empty())
      continue;
    rects.insert(RoundBounds(rect));
  }
  return rects;
}

inline void FillRect(gfx::Canvas* canvas, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(*canvas, color);
  canvas->FillRectangle(fill_brush, rect);
}

} // namespace

typedef std::list<TextLine*>::const_iterator FormatLineIterator;

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock::Caret
//
class ScreenTextBlock::Caret {
  private: gfx::RectF bounds_;
  private: base::Time last_blink_time_;
  private: bool has_focus_;
  private: bool shown_;

  public: Caret();
  public: ~Caret();

  public: bool has_focus() const { return has_focus_; }

  public: void Blink(gfx::Canvas* canvas, const gfx::RectF& clip_bounds);
  public: void DidKillFocus();
  public: void DidPaint(const gfx::RectF& bounds);
  public: void DidSetFocus();
  public: void Hide(gfx::Canvas* canvas);
  public: void HideIfNeeded(gfx::Canvas* canvas);
  public: void Show(gfx::Canvas* canvas);
  public: void Update(gfx::Canvas* canvas, const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

ScreenTextBlock::Caret::Caret() : has_focus_(false), shown_(false) {
}

ScreenTextBlock::Caret::~Caret() {
}

void ScreenTextBlock::Caret::Blink(gfx::Canvas* canvas,
                                   const gfx::RectF& clip_bounds) {
  if (!has_focus_) {
    if (shown_)
      Hide(canvas);
    return;
  }
  static const auto kBlinkInterval = 500; // milliseconds
  auto const now = base::Time::Now();
  auto const delta = now - last_blink_time_;
  if (delta < base::TimeDelta::FromMilliseconds(kBlinkInterval))
    return;
  last_blink_time_ = now;
  gfx::Canvas::DrawingScope drawing_scope(*canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas, clip_bounds);
  if (shown_)
    Hide(canvas);
  else
    Show(canvas);
}

void ScreenTextBlock::Caret::DidKillFocus() {
  DCHECK(has_focus_);
  has_focus_ = false;
}

void ScreenTextBlock::Caret::DidPaint(const gfx::RectF& bounds) {
  if (!bounds.Contains(bounds_))
    return;
  shown_ = false;
}

void ScreenTextBlock::Caret::DidSetFocus() {
  DCHECK(!has_focus_);
  has_focus_ = true;
}

void ScreenTextBlock::Caret::Hide(gfx::Canvas* canvas) {
  DCHECK(shown_);
  if (!has_focus_ || bounds_.empty())
    return;
  shown_ = false;
  canvas->DrawBitmap(*canvas->screen_bitmap(), bounds_, bounds_);
  canvas->set_dirty_rect(bounds_);
}

void ScreenTextBlock::Caret::HideIfNeeded(gfx::Canvas* canvas) {
  if (!shown_)
    return;
  Hide(canvas);
}

void ScreenTextBlock::Caret::Show(gfx::Canvas* canvas) {
  DCHECK(!shown_);
  if (!has_focus_ || bounds_.empty())
    return;
  shown_ = true;
  gfx::Brush caret_brush(*canvas, gfx::ColorF::Black);
  canvas->FillRectangle(caret_brush, bounds_);
  canvas->set_dirty_rect(bounds_);
}

void ScreenTextBlock::Caret::Update(gfx::Canvas* canvas,
                                    const gfx::RectF& new_bounds) {
  DCHECK(has_focus_);
  DCHECK(!shown_);
  bounds_ = new_bounds;
  Show(canvas);
  last_blink_time_ = base::Time::Now();
  ui::TextInputClient::Get()->set_caret_bounds(bounds_);
}

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock::RenderContext
//
class ScreenTextBlock::RenderContext {
  private: const gfx::ColorF bgcolor_;
  private: mutable std::vector<gfx::RectF> copy_rects_;
  private: mutable std::vector<gfx::RectF> dirty_rects_;
  private: const std::list<TextLine*>& format_lines_;
  private: gfx::Canvas* canvas_;
  private: const gfx::RectF bounds_;
  private: const std::vector<TextLine*>& screen_lines_;
  private: const ScreenTextBlock* screen_text_block_;
  private: mutable std::vector<gfx::RectF> skip_rects_;

  public: RenderContext(const ScreenTextBlock* screen_text_block,
                        const TextBlock* format_text_block);
  public: ~RenderContext() = default;

  private: void Copy(float dst_top, float dst_bottom, float src_top) const;
  private: void DrawDirtyRect(const gfx::RectF& rect, float red, float green,
                              float blue) const;
  private: void FillBottom(const TextLine* line) const;
  private: void FillRight(const TextLine* line) const;
  private: FormatLineIterator FindFirstMismatch() const;
  private: FormatLineIterator FindLastMatch() const;
  private: std::vector<TextLine*>::const_iterator FindCopyable(
      TextLine* line) const;
  public: void Finish();
  public: bool Render();
  private: void RestoreSkipRect(const gfx::RectF& rect) const;
  public: FormatLineIterator TryCopy(
      const FormatLineIterator& format_line_start,
      const FormatLineIterator& format_line_end) const;

  DISALLOW_COPY_AND_ASSIGN(RenderContext);
};

ScreenTextBlock::RenderContext::RenderContext(
    const ScreenTextBlock* screen_text_block,
    const TextBlock* format_text_block)
    : bgcolor_(format_text_block->default_style().bgcolor()),
      format_lines_(format_text_block->lines()),
      canvas_(screen_text_block->canvas_), bounds_(screen_text_block->bounds_),
      screen_text_block_(screen_text_block),
      screen_lines_(screen_text_block->lines_) {
}

void ScreenTextBlock::RenderContext::Copy(float dst_top, float dst_bottom,
                                          float src_top) const {
  auto const height = dst_bottom - dst_top;
  DCHECK_GT(height, 0.0f);
  DCHECK_LE(src_top + height, bounds_.bottom);

  gfx::RectF dst_rect(bounds_.left, dst_top, bounds_.right, dst_top + height);
  gfx::RectF src_rect(bounds_.left, src_top, bounds_.right, src_top + height);
  DCHECK_EQ(dst_rect.size(), src_rect.size());
  #if DEBUG_DRAW
    DVLOG(0) << "Copy to " << dst_rect << " from " << src_rect.left_top();
  #endif
  canvas_->DrawBitmap(*canvas_->screen_bitmap(), dst_rect, src_rect);
}

void ScreenTextBlock::RenderContext::DrawDirtyRect(
    const gfx::RectF& rect, float red, float green, float blue) const {
  RestoreSkipRect(rect);
  if (views::switches::text_window_display_paint) {
    #if USE_OVERLAY
      canvas_->FillRectangle(gfx::Brush(*canvas_, red, green, blue, 0.1f), rect);
      canvas_->DrawRectangle(gfx::Brush(*canvas_, red, green, blue, 0.5f), rect,
                          0.5f);
    #else
      auto marker_rect = rect;
      marker_rect.left += kMarkerLeftMargin;
      marker_rect.right = marker_rect.left + kMarkerWidth;
      canvas_->FillRectangle(gfx::Brush(*canvas_, red, green, blue), marker_rect);
    #endif
  }
}

void ScreenTextBlock::RenderContext::FillBottom(const TextLine* line) const {
  gfx::RectF rect(bounds_);;
  rect.top = line->bottom();
  if (rect.empty())
    return;
  gfx::Brush fill_brush(*canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
}

void ScreenTextBlock::RenderContext::FillRight(const TextLine* line) const {
  gfx::RectF rect(line->bounds());
  rect.left  = bounds_.left + line->GetWidth();
  rect.right = bounds_.right;
  if (rect.empty())
    return;
  gfx::Brush fill_brush(*canvas_, bgcolor_);
  canvas_->FillRectangle(fill_brush, rect);
}

FormatLineIterator ScreenTextBlock::RenderContext::FindFirstMismatch() const {
  auto screen_line_runner = screen_lines_.cbegin();
  for (auto format_line_runner = format_lines_.cbegin();
       format_line_runner != format_lines_.cend();
       ++format_line_runner) {
    if (screen_line_runner == screen_lines_.cend())
      return format_line_runner;
    auto const format_line = (*format_line_runner);
    auto const screen_line = (*screen_line_runner);
    if (format_line->bounds() != screen_line->bounds() ||
        !format_line->Equal(screen_line)) {
      return format_line_runner;
    }
    AddRect(skip_rects_, format_line->bounds());
    ++screen_line_runner;
  }
  return format_lines_.cend();
}

FormatLineIterator ScreenTextBlock::RenderContext::FindLastMatch() const {
  auto screen_line_runner = screen_lines_.crbegin();
  auto format_last_match = format_lines_.crbegin();
  for (auto format_line_runner = format_lines_.crbegin();
       format_line_runner != format_lines_.crend();
       ++format_line_runner) {
    auto const format_line = (*format_line_runner);
    if (format_line->top() >= bounds_.bottom)
      continue;
    if (screen_line_runner == screen_lines_.crend())
      break;
    auto const screen_line = (*screen_line_runner);
    if (format_line->bounds() != screen_line->bounds() ||
        !format_line->Equal(screen_line)) {
      break;
    }
    AddRect(skip_rects_, format_line->bounds());
    format_last_match = format_line_runner;
    ++screen_line_runner;
  }

  return format_last_match == format_lines_.crbegin() ? format_lines_.end() :
      std::find(format_lines_.cbegin(), format_lines_.cend(),
                *format_last_match);
}

std::vector<TextLine*>::const_iterator
    ScreenTextBlock::RenderContext::FindCopyable(TextLine* format_line) const {
  for (auto runner = screen_lines_.begin(); runner != screen_lines_.end();
       ++runner) {
    auto const screen_line = *runner;
    if (screen_line->Equal(format_line) &&
        (format_line->bounds().top == screen_line->bounds().top ||
         screen_line->bounds().bottom <= bounds_.bottom)) {
      return runner;
    }
  }
  return screen_lines_.end();
}

void ScreenTextBlock::RenderContext::Finish() {
  // Draw dirty rectangles for debugging.
  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas_, bounds_);
  for (auto rect : copy_rects_) {
    #if DEBUG_DRAW
      DVLOG(0) << "copy " << rect;
    #endif
    DrawDirtyRect(rect, 58.0f / 255, 128.0f / 255, 247.0f / 255);
  }
  for (auto rect : dirty_rects_) {
    #if DEBUG_DRAW
      DVLOG(0) << "dirty " << rect;
    #endif
    DrawDirtyRect(rect, 219.0f / 255, 68.0f / 255, 55.0f / 255);
  }
  canvas_->Flush();
}

bool ScreenTextBlock::RenderContext::Render() {
  #if DEBUG_DRAW
    DVLOG(0) << "Start rendering";
  #endif

  if (VLOG_IS_ON(0)) {
    // TextBlock must cover whole screen area.
    auto const last_format_line = format_lines_.back();
    if (!last_format_line->cells().back()->is<MarkerCell>())
      DCHECK_GE(last_format_line->bounds().bottom, bounds_.bottom);
  }

  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas_, bounds_);

  auto const dirty_line_start = FindFirstMismatch();
  if (dirty_line_start != format_lines_.end()) {
    auto const clean_line_start = FindLastMatch();
    #if DEBUG_DRAW
      DVLOG(0) << "dirty " << (*dirty_line_start)->bounds().top << "," <<
        (clean_line_start == format_lines_.end() ? bounds_.bottom :
            (*clean_line_start)->bounds().top);
    #endif
    for (auto dirty_line_runner = dirty_line_start;
         dirty_line_runner != clean_line_start;
         ++dirty_line_runner) {
      dirty_line_runner = TryCopy(dirty_line_runner, clean_line_start);
      if (dirty_line_runner == clean_line_start)
        break;
      auto const format_line = *dirty_line_runner;
      format_line->Render(*canvas_);
      FillRight(format_line);
      AddRect(dirty_rects_, format_line->bounds());
    }
  }

  // Erase dirty rectangle markers.
  for (auto rect : skip_rects_) {
    #if DEBUG_DRAW
      DVLOG(0) << "skip " << rect;
    #endif
    RestoreSkipRect(rect);
  }
  FillBottom(format_lines_.back());

  auto const dirty = !copy_rects_.empty() || !dirty_rects_.empty();
  #if DEBUG_DRAW
    DVLOG(0) << "End rendering dirty=" << dirty;
  #endif
  return dirty;
}

void ScreenTextBlock::RenderContext::RestoreSkipRect(
    const gfx::RectF& rect) const {
  auto marker_rect = rect;
  marker_rect.left += kMarkerLeftMargin;
  marker_rect.right = marker_rect.left + kMarkerWidth;
  canvas_->FillRectangle(gfx::Brush(*canvas_, gfx::ColorF::White), marker_rect);
  if (!screen_text_block_->has_screen_bitmap_)
    return;
  auto const line_rect = gfx::RectF(gfx::PointF(bounds_.left, rect.top),
                                    gfx::SizeF(bounds_.width(), rect.height()));
  canvas_->DrawBitmap(*canvas_->screen_bitmap(), line_rect, line_rect);
}

FormatLineIterator ScreenTextBlock::RenderContext::TryCopy(
    const FormatLineIterator& format_current,
    const FormatLineIterator& format_end) const {
  if (!screen_text_block_->has_screen_bitmap_)
    return format_current;

  auto const screen_end = screen_lines_.end();
  auto format_runner = format_current;
  while (format_runner != format_end) {
    // TODO(yosi) Should we search longest match? How?
    auto const format_start = format_runner;
    auto const screen_start = FindCopyable(*format_start);
    if (screen_start == screen_lines_.end())
      return format_runner;

    auto const dst_top = (*format_start)->top();
    auto const src_top = (*screen_start)->top();

    auto const skip = dst_top == src_top;
    if (skip)
      AddRect(skip_rects_, (*format_start)->bounds());
    else
      AddRect(copy_rects_, (*format_start)->bounds());

    auto dst_bottom = (*format_start)->bottom();
    ++format_runner;
    auto screen_runner = screen_start;
    ++screen_runner;

    while (format_runner != format_end && screen_runner != screen_end) {
      auto const format_line = *format_runner;
      auto const screen_line = *screen_runner;
      if (screen_line->bounds().bottom > bounds_.bottom ||
          !format_line->Equal(screen_line))
        break;
      if (skip)
        AddRect(skip_rects_, format_line->bounds());
      else
        AddRect(copy_rects_, format_line->bounds());
      dst_bottom = format_line->bottom();
      ++format_runner;
      ++screen_runner;
    }

    if (!skip)
      Copy(dst_top, dst_bottom, src_top);
  }
  return format_runner;
}

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock
//
ScreenTextBlock::ScreenTextBlock()
    : canvas_(nullptr), caret_(new Caret()), dirty_(true),
      has_screen_bitmap_(false) {
}

ScreenTextBlock::~ScreenTextBlock() {
}

void ScreenTextBlock::DidKillFocus() {
  caret_->DidKillFocus();
}

void ScreenTextBlock::DidSetFocus() {
  caret_->DidSetFocus();
}

gfx::RectF ScreenTextBlock::HitTestTextPosition(text::Posn offset) const {
  if (offset < lines_.front()->text_start() ||
      offset > lines_.back()->text_end()) {
    return gfx::RectF();
  }
  for (auto const line : lines_) {
    auto const rect = line->HitTestTextPosition(offset);
    if (!rect.empty())
      return RoundBounds(rect);
  }
  return gfx::RectF();
}

void ScreenTextBlock::Render(const TextBlock* text_block,
                             const TextSelection& selection) {
  RenderContext render_context(this, text_block);
  dirty_ = render_context.Render();
  caret_->DidPaint(bounds_);
  if (!dirty_) {
    RenderSelection(selection);
    return;
  }

  Reset();
  has_screen_bitmap_ = canvas_->SaveScreenImage(bounds_);
  // Event if we can't get bitmap from render target, screen is up-to-date,
  // but we render all lines next time.
  if (has_screen_bitmap_) {
    // TODO(yosi) We should use existing TextLine's in ScreenTextBlock.
    for (auto line : text_block->lines()) {
      lines_.push_back(line->Copy());
    }
  }
  render_context.Finish();
  RenderSelection(selection);
  dirty_ = false;
}

void ScreenTextBlock::RenderCaret() {
  if (!caret_->has_focus())
    return;
  auto const char_rect = HitTestTextPosition(selection_.active_offset());
  if (char_rect.empty())
    return;
  auto const caret_width = 2;
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          char_rect.left + caret_width, char_rect.bottom);
  caret_->Update(canvas_, caret_bounds);
}

void ScreenTextBlock::RenderSelection(const TextSelection& selection) {
  selection_ = selection;
  if (selection_.start() >= lines_.back()->text_end())
    return;
  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas_, bounds_);
  if (selection.is_range() &&
      selection_.end() > lines_.front()->text_start()) {
    gfx::Brush fill_brush(*canvas_, selection_.color());
    for (auto line : lines_) {
      if (selection_.end() <= line->text_start())
          break;
      auto const rect = line->CalculateSelectionRect(selection);
      if (rect.empty())
        continue;
      canvas_->FillRectangle(fill_brush, rect);
    }
  }
  RenderCaret();
}

void ScreenTextBlock::RenderSelectionIfNeeded(
    const TextSelection& new_selection) {
  if (selection_ == new_selection) {
    caret_->Blink(canvas_, bounds_);
    return;
  }
  auto min_left = bounds_.right;
  auto max_right = bounds_.left;
  auto new_selection_rects = CalculateSelectionRects(lines_, new_selection);
  for (const auto& rect : new_selection_rects) {
    min_left = std::min(min_left, rect.left);
    max_right = std::max(max_right, rect.right);
  }
  auto old_selection_rects = CalculateSelectionRects(lines_, selection_);
  for (const auto& rect : old_selection_rects) {
    min_left = std::min(min_left, rect.left);
    max_right = std::max(max_right, rect.right);
  }
  if (old_selection_rects.empty() && new_selection_rects.empty() &&
      !caret_->has_focus()) {
    selection_ = new_selection;
    return;
  }

  gfx::Canvas::DrawingScope drawing_scope(*canvas_);
  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas_, bounds_);
  for (const auto& old_rect : old_selection_rects) {
    if (new_selection_rects.find(old_rect) != new_selection_rects.end())
      continue;
    canvas_->set_dirty_rect(old_rect);
    canvas_->DrawBitmap(*canvas_->screen_bitmap(), old_rect, old_rect);
    caret_->DidPaint(old_rect);
  }

  if (selection_.color() != new_selection.color())
    old_selection_rects.clear();
  if (!new_selection_rects.empty()) {
    gfx::Brush fill_brush(*canvas_, new_selection.color());
    for (const auto& new_rect : new_selection_rects) {
      if (old_selection_rects.find(new_rect) != old_selection_rects.end())
        continue;
      canvas_->set_dirty_rect(new_rect);
      canvas_->DrawBitmap(*canvas_->screen_bitmap(), new_rect, new_rect);
      canvas_->FillRectangle(fill_brush, new_rect);
      caret_->DidPaint(new_rect);
    }
  }

  caret_->HideIfNeeded(canvas_);
  selection_ = new_selection;
  RenderCaret();
}

void ScreenTextBlock::Reset() {
  dirty_ = true;
  for (auto line : lines_) {
    delete line;
  }
  lines_.clear();
  has_screen_bitmap_ = false;
}

void ScreenTextBlock::SetBounds(const gfx::RectF& rect) {
  Reset();
  bounds_ = rect;
}

void ScreenTextBlock::SetCanvas(gfx::Canvas* canvas) {
  Reset();
  canvas_ = canvas;
}

// gfx::Canvas::Observer
void ScreenTextBlock::ShouldDiscardResources() {
  Reset();
}

} // namespace rendering
} // namespace views
