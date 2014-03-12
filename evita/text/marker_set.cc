// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker_set.h"

#include <vector>

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// MarkerSet::ChangeScope
//
class MarkerSet::ChangeScope {
  private: MarkerSetImpl* markers_;
  private: std::vector<Marker*> markers_to_remove_;

  public: ChangeScope(MarkerSetImpl* markers);
  public: ~ChangeScope();

  public: void Remove(Marker* marker);

  DISALLOW_COPY_AND_ASSIGN(ChangeScope);
};

MarkerSet::ChangeScope::ChangeScope(MarkerSetImpl* markers)
    : markers_(markers) {
}

MarkerSet::ChangeScope::~ChangeScope() {
  for (auto const marker : markers_to_remove_) {
    markers_->erase(marker);
    delete marker;
  }
}

void MarkerSet::ChangeScope::Remove(Marker* marker) {
  markers_to_remove_.push_back(marker);
}

//////////////////////////////////////////////////////////////////////
//
// MarkerSet
//
MarkerSet::MarkerSet() {
}

MarkerSet::~MarkerSet() {
}

MarkerSet::MarkerSetImpl::iterator MarkerSet::lower_bound(Posn offset) {
  Marker marker(offset);
  return markers_.lower_bound(&marker);
}

void MarkerSet::AddObserver(MarkerSetObserver* observer) {
  observers_.AddObserver(observer);
}

void MarkerSet::Clear() {
  if (markers_.empty())
    return;
  auto const start = (*markers_.begin())->start_;
  auto const end = (*markers_.rbegin())->end_;
  for (auto marker : markers_) {
    delete marker;
  }
  markers_.clear();
  NotifyChange(start, end);
}

Marker MarkerSet::GetMarkerAt(Posn offset) {
  if (markers_.empty())
    return Marker();
  auto const present = lower_bound(offset + 1);
  if (present == markers_.end())
    return Marker();
  DCHECK_LT(offset, (*present)->end_);
  return offset >= (*present)->start_ ? **present : Marker();
}

void MarkerSet::InsertMarker(Posn start, Posn end, int type) {
  DCHECK_LT(start, end);
  RemoveMarker(start, end);

  auto const after = lower_bound(end);
  auto const can_merge_after = after != markers_.end() &&
      (*after)->type_ == type && (*after)->start_ == end;
  auto const before = lower_bound(start);
  auto const can_merge_before = before != markers_.end() &&
      (*before)->type_ == type && (*before)->end_ == start;

  ChangeScope change_scope(&markers_);
  NotifyChange(start, end);
  if (can_merge_after && can_merge_before) {
    (*before)->end_ = (*after)->end_;
    change_scope.Remove(*after);
    return;
  }
  if (can_merge_after) {
    (*after)->start_ = start;
    return;
  }
  if (can_merge_before) {
    (*before)->end_ = end;
    return;
  }
  markers_.insert(new Marker(start, end, type));
}

void MarkerSet::NotifyChange(Posn start, Posn end) {
  FOR_EACH_OBSERVER(MarkerSetObserver, observers_,
      DidChangeMarker(start, end));
}

void MarkerSet::RemoveMarker(Posn start, Posn end) {
  DCHECK_LT(start, end);
  if (markers_.empty())
    return;

  ChangeScope change_scope(&markers_);

  auto offset = start;
  auto runner = lower_bound(offset + 1);
  while (offset < end) {
    if (runner == markers_.end() || (*runner)->start_ >= end)
      return;

    auto const runner_end = (*runner)->end_;
    auto const runner_start = (*runner)->start_;
    auto const runner_type = (*runner)->type_;

    if (runner_end >= end) {
      if (runner_start <= offset) {
        // before: --xxxxxxxx--
        // new:    ----____----
        // after:  --xx____xx--
        NotifyChange(offset, end);
        (*runner)->start_ = end;
        if (runner_start < offset)
          markers_.insert(new Marker(runner_start, offset, runner_type));
        return;
      }

      // before: ----xxxx--
      // new:    --____----
      // after:  ----__xx--
      if (runner_end == end)
        change_scope.Remove(*runner);
      else
        (*runner)->start_ = end;
      NotifyChange(start, end);
      return;
    }

    if (runner_start < offset) {
      // before: --xxxxx??--
      // new:    ----_____--
      // after:  --xx___??--
      NotifyChange(offset, runner_end);
      (*runner)->end_ = offset;
    } else if (runner_start == offset) {
      // before: --xxxx??--
      // new:    --_____--
      // after:  --____??--
      NotifyChange(offset, runner_end);
      change_scope.Remove(*runner);
    } else {
      // before: ----xxxx??--
      // new:    --________--
      // after:  ----____??--
      NotifyChange(offset, runner_end);
      change_scope.Remove(*runner);
    }

    offset = (*runner)->end_;
    ++runner;
  }
}

void MarkerSet::RemoveObserver(MarkerSetObserver* observer) {
  observers_.RemoveObserver(observer);
}

// BufferMutationObserver
void MarkerSet::DidDeleteAt(Posn offset, size_t length) {
  ChangeScope change_scope(&markers_);
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ > offset) {
      marker->start_ = std::max(static_cast<Posn>(marker->start_ - length),
                                offset);
    }
    if (marker->end_ > offset) {
      marker->end_ = std::max(static_cast<Posn>(marker->start_ - length),
                              offset);
    }
    if (marker->start_ == marker->end_)
      change_scope.Remove(marker);
  }
}

void MarkerSet::DidInsertAt(Posn offset, size_t length) {
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ > offset)
      marker->start_ += length;
    marker->end_ += length;
  }
}

void MarkerSet::DidInsertBefore(Posn offset, size_t length) {
  for (auto runner = lower_bound(offset + 1); runner != markers_.end();
       ++runner) {
    auto const marker = *runner;
    if (marker->start_ >= offset)
      marker->start_ += length;
    marker->end_ += length;
  }
}

}  // namespace text
