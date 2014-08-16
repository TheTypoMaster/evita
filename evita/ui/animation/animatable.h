// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_animation_animatable_h)
#define INCLUDE_evita_ui_animation_animatable_h

#include "base/time/time.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)

namespace base {
class Time;
}

namespace ui {

class AnimationObserver;

class Animatable {
  private: ObserverList<AnimationObserver> observers_;

  protected: Animatable();
  public: virtual ~Animatable();

  public: void AddObserver(AnimationObserver* observer);
  protected: virtual void Animate(base::Time time) = 0;
  protected: void DidAnimate();
  public: void FinishAnimation();
  public: void PlayAnimation(base::Time time);
  public: void RemoveObserver(AnimationObserver* observer);

  DISALLOW_COPY_AND_ASSIGN(Animatable);
};

}   // namespace ui

#endif //!defined(INCLUDE_evita_ui_animation_animatable_h)
