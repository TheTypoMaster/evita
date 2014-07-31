// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_size_h)
#define INCLUDE_common_win_size_h

#include "common/common_export.h"

namespace common {
namespace win {

struct COMMON_EXPORT Size : SIZE {
  Size() {
    cx = cy = 0;
  }

  Size(int cx, int cy) {
    this->cx = cx;
    this->cy = cy;
  }

  explicit operator bool() const { return !empty(); }

  bool operator==(const Size& other) const;
  bool operator!=(const Size& other) const;

  bool empty() const { return cx <= 0 || cx <= 0; }
};

inline bool Size::operator==(const Size& other) const {
  return cx == other.cx && cy == other.cy;
}

inline bool Size::operator!=(const Size& other) const {
  return !operator==(other);
}

} // namespace win
} // namespace common

#include <ostream>

COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size& size);
COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const common::win::Size* size);

#endif //!defined(INCLUDE_common_win_size_h)
