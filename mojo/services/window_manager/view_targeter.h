// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGETER_H_
#define MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGETER_H_

#include "ui/events/event_targeter.h"

namespace mojo {

class ViewTargeter : public ui::EventTargeter {
 public:
  ViewTargeter();
  ~ViewTargeter() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewTargeter);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGETER_H_
