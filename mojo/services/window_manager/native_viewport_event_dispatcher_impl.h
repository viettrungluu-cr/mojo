// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_NATIVE_VIEWPORT_EVENT_DISPATCHER_IMPL_H_
#define MOJO_SERVICES_WINDOW_MANAGER_NATIVE_VIEWPORT_EVENT_DISPATCHER_IMPL_H_

#include "base/basictypes.h"
#include "mojo/services/public/interfaces/native_viewport/native_viewport.mojom.h"
#include "ui/events/event_source.h"

namespace mojo {

class WindowManagerApp;

class NativeViewportEventDispatcherImpl
    : public ui::EventSource,
      public InterfaceImpl<NativeViewportEventDispatcher> {
 public:
  explicit NativeViewportEventDispatcherImpl(WindowManagerApp* app);
  ~NativeViewportEventDispatcherImpl() override;

 private:
  // ui::EventSource:
  ui::EventProcessor* GetEventProcessor() override;

  // NativeViewportEventDispatcher:
  void OnEvent(mojo::EventPtr event,
               const mojo::Callback<void()>& callback) override;

  WindowManagerApp* app_;

  DISALLOW_COPY_AND_ASSIGN(NativeViewportEventDispatcherImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_NATIVE_VIEWPORT_EVENT_DISPATCHER_IMPL_H_
