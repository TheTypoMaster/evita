// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_error_h)
#define INCLUDE_evita_dom_public_io_error_h

namespace domapi {

struct IoError {
  int error_code;

  explicit IoError(int error_code);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_error_h)
