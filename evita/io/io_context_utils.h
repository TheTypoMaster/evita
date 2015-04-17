// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_context_utils_h)
#define INCLUDE_evita_io_io_context_utils_h

#include <stdint.h>

#include "base/callback_forward.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"

namespace io {

void Reject(const base::Callback<void(domapi::IoError)>& reject,
            uint32_t last_error);

void Resolve(const base::Callback<void(int)>& resolve,
             uint32_t num_transferred);

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_context_utils_h)
