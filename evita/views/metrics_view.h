// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_METRICS_VIEW_H_
#define EVITA_VIEWS_METRICS_VIEW_H_

#include <memory>

#include "base/time/time.h"
#include "evita/ui/widget.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// MetricsView
//
class MetricsView final : public ui::Widget {
 public:
  class TimingScope final {
   public:
    explicit TimingScope(MetricsView* view);
    ~TimingScope();

   private:
    base::TimeTicks start_;
    MetricsView* view_;

    DISALLOW_COPY_AND_ASSIGN(TimingScope);
  };

  MetricsView();
  ~MetricsView() final;

  void Animate(base::Time now);
  void RecordTime();

 private:
  class View;

  // ui::Widget
  void DidRealize() final;

  std::unique_ptr<View> view_;

  DISALLOW_COPY_AND_ASSIGN(MetricsView);
};
}  // namespace views

#endif  // EVITA_VIEWS_METRICS_VIEW_H_
