// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animatable_window_h)
#define INCLUDE_evita_ui_animatable_window_h

#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/widget.h"

namespace ui {

class AnimatableWindow : public AnimationFrameHandler, public Widget {
  DECLARE_CASTABLE_CLASS(AnimatableWindow, Widget);

  protected: explicit AnimatableWindow(
      std::unique_ptr<NativeWindow> native_window);
  protected: AnimatableWindow();
  protected: virtual ~AnimatableWindow();

  // AnimationFrameHanndler
  protected: virtual void DidBeginAnimationFrame(base::Time time) override;

  // Widget
  protected: virtual void DidChangeBounds() override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidShow() override;
  protected: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(AnimatableWindow);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_animatable_window_h)
