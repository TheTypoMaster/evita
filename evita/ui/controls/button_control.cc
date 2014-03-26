// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/button_control.h"

#include "evita/gfx_base.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ButtonControl::Renderer
//
class ButtonControl::Renderer {
  private: gfx::RectF rect_;
  private: Style style_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Renderer(const base::string16& text, const Style& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Graphics* gfx, Control::State state) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const ButtonControl::Style& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  common::ComPtr<IDWriteInlineObject> inline_object;
  COM_VERIFY(gfx::FactorySet::instance()->dwrite().
      CreateEllipsisTrimmingSign(text_format, &inline_object));
  DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
  text_format->SetTrimming(&trimming, inline_object);
  return text_format.CreateLayout(text, size);
}

gfx::ColorF MakeColorWithAlpha(const gfx::ColorF color, float alpha) {
  return gfx::ColorF(color.red(), color.green(), color.blue(), alpha);
}
}  // namespace

ButtonControl::Renderer::Renderer(const base::string16& text,
                                  const Style& style,
                                  const gfx::RectF& rect)
    : rect_(rect), style_(style),
      text_layout_(CreateTextLayout(text, style, rect.size())) {
}

ButtonControl::Renderer::~Renderer() {
}

void ButtonControl::Renderer::Render(gfx::Graphics* gfx,
                                     Control::State state) const {
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, rect_);
  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect_);
  gfx->DrawRectangle(gfx::Brush(*gfx, style_.shadow), rect_);

  auto const max_width = (*text_layout_)->GetMaxWidth();
  auto const max_height = (*text_layout_)->GetMaxHeight();
  gfx::PointF origin(rect_.left + (rect_.width() - max_width) / 2.0f,
                     rect_.top + (rect_.height() - max_height) / 2.0f);
  gfx::Brush text_brush(*gfx, state == State::Disabled ?
      style_.gray_text : style_.color);
  (*gfx)->DrawTextLayout(origin, *text_layout_, text_brush,
                         D2D1_DRAW_TEXT_OPTIONS_CLIP);

  switch (state) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      gfx->FillRectangle(
          gfx::Brush(*gfx, MakeColorWithAlpha(style_.highlight, 0.3f)),
          rect_);
      break;
    case Control::State::Hover:
      gfx->DrawRectangle(
          gfx::Brush(*gfx, MakeColorWithAlpha(style_.highlight, 0.3f)),
          rect_);
      break;
  }
  gfx->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
ButtonControl::ButtonControl(ControlController* controller,
                           const base::string16& text, const Style& style)
    : Control(controller), style_(style), text_(text) {
}

ButtonControl::~ButtonControl() {
}

void ButtonControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  renderer_.reset();
}

void ButtonControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  renderer_.reset();
}

// ui::Widget
void ButtonControl::DidResize() {
  renderer_.reset();
}

void ButtonControl::OnDraw(gfx::Graphics* gfx) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, gfx::RectF(rect()));
  renderer_->Render(gfx, state());
}

}  // namespace ui
