// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_form_observer_h)
#define INCLUDE_evita_dom_forms_form_observer_h

#include "base/macros.h"

namespace dom {

class FormObserver {
  protected: FormObserver();
  public: virtual ~FormObserver();

  public: virtual void DidChangeForm() = 0;

  DISALLOW_COPY_AND_ASSIGN(FormObserver);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_observer_h)
