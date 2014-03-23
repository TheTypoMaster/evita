// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_form_window_h)
#define INCLUDE_evita_views_forms_form_window_h

#include "evita/views/window.h"

#include <memory>

#include "evita/gc/member.h"

namespace dom {
class Form;
}

namespace gfx {
class Graphics;
using common::win::Rect;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow : public views::Window {
  DECLARE_CASTABLE_CLASS(FormWindow, views::Window);

  private: class FormViewModel;

  private: gc::Member<const dom::Form> form_;
  private: private: std::unique_ptr<gfx::Graphics> gfx_;
  private: private: std::unique_ptr<FormViewModel> model_;
  private: gfx::Rect pending_update_rect_;

  public: FormWindow(views::WindowId window_id, const dom::Form* form);
  public: virtual ~FormWindow();

  public: static bool DoIdle(int hint);
  private: bool OnIdle(int hint);

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidResize() override;
  private: virtual LRESULT OnMessage(uint32_t message, WPARAM wParam,
                                     LPARAM lParam) override;
  private: virtual void OnPaint(const gfx::Rect paint_rect) override;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_forms_form_window_h)