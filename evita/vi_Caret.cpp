#include "precomp.h"
// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/vi_Caret.h"

#include <math.h>
#include <memory>
#include <utility>

#include "evita/gfx_base.h"

static const auto kBlinkInterval = 500; // milliseconds

//////////////////////////////////////////////////////////////////////
//
// BackingStore
//
class Caret::BackingStore {
  private: std::unique_ptr<gfx::Bitmap> bitmap_;
  private: void* render_target_;
  private: gfx::RectF rect_;

  public: BackingStore();
  public: ~BackingStore();
  public: void Restore(const gfx::Graphics& gfx);
  public: void Save(const gfx::Graphics& gfx, const gfx::RectF& rect);

  DISALLOW_COPY_AND_ASSIGN(BackingStore);
};

Caret::BackingStore::BackingStore()
    : render_target_(nullptr) {
}

Caret::BackingStore::~BackingStore() {
}

void Caret::BackingStore::Restore(const gfx::Graphics& gfx) {
  if (!rect_ || render_target_ != &*gfx || !bitmap_)
    return;
  DCHECK(bitmap_);
  gfx->DrawBitmap(*bitmap_, rect_);
  gfx->Flush();
}

void Caret::BackingStore::Save(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) {
  DCHECK(!rect.is_empty());
  rect_ = gfx::RectF(::floorf(rect.left), ::floorf(rect.top),
                     ::ceilf(rect.right), ::ceilf(rect.bottom));
  gfx::RectU screen_rect = gfx::RectU(static_cast<uint>(rect_.left),
                                      static_cast<uint>(rect_.top),
                                      static_cast<uint>(rect_.right),
                                      static_cast<uint>(rect_.bottom));
  std::unique_ptr<gfx::Bitmap> bitmap(
      new gfx::Bitmap(gfx, screen_rect.size()));
  if (!bitmap)
    return;
  COM_VERIFY((*bitmap)->CopyFromRenderTarget(nullptr, gfx, &screen_rect));
  bitmap_ = std::move(bitmap);
  render_target_ = &*gfx;
}

//////////////////////////////////////////////////////////////////////
//
// Caret
//
Caret::Caret()
  : backing_store_(new BackingStore()),
    blink_timer_(this, &Caret::Blink),
    gfx_(nullptr),
    shown_(false),
    should_blink_(false),
    taken_(false) {
}

Caret::~Caret() {
  DCHECK(!taken_);
}

void Caret::Blink(common::RepeatingTimer<Caret>*) {
  if (!taken_ || !should_blink_ || !rect_)
    return;
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  if (shown_)
    Hide();
  else
    Show();
}

std::unique_ptr<Caret> Caret::Create() {
  return std::move(std::unique_ptr<Caret>(new Caret()));
}

void Caret::Give() {
  DCHECK(taken_);
  blink_timer_.Stop();
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  Hide();
  taken_ = false;
}

void Caret::Hide() {
  if (!taken_)
    return;
  if (!shown_)
    return;
  backing_store_->Restore(*gfx_);
  shown_ = false;
}

void Caret::Reset() {
  DCHECK(!shown_);
  should_blink_ = false;
  rect_ = gfx::RectF();
}

void Caret::Show() {
  DCHECK(rect_);
  if (shown_)
    return;
  backing_store_->Save(*gfx_, rect_);
  gfx::Brush fill_brush(*gfx_, gfx::ColorF::Black);
  gfx_->FillRectangle(fill_brush, rect_);
  shown_ = true;
}

void Caret::Take(const gfx::Graphics& gfx) {
  taken_ = true;
  gfx_ = &gfx;
  blink_timer_.Start(kBlinkInterval);
  if (!rect_)
    return;
  gfx::Graphics::DrawingScope drawing_scope(*gfx_);
  Show();
}

void Caret::Update(const gfx::RectF& new_rect) {
  DCHECK(new_rect);
  DCHECK(taken_);
  DCHECK(!shown_);
  if (rect_ == new_rect) {
    should_blink_ = true;
  } else {
    rect_ = new_rect;
    should_blink_ = false;
  }
  Show();
}
