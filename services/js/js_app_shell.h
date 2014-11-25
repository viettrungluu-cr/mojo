// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_JS_JS_APP_SHELL_H_
#define MOJO_SERVICES_JS_JS_APP_SHELL_H_

#include "gin/handle.h"
#include "gin/wrappable.h"
#include "mojo/edk/js/handle.h"

namespace mojo {
namespace js {

class JSApp;

class JSAppShell : public gin::Wrappable<JSAppShell> {
 public:
  static gin::WrapperInfo kWrapperInfo;
  static gin::Handle<JSAppShell> Create(v8::Isolate* isolate, JSApp* js_app);

 private:
  explicit JSAppShell(JSApp* js_app);
  ~JSAppShell() override;

  // gin::WrappableBase
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate* isolate)
      override;

  // Bound to JS method "connectToApplication".
  void ConnectToApplication(
      const std::string& application_url, mojo::Handle service_provider);

  JSApp* js_app_;

  DISALLOW_COPY_AND_ASSIGN(JSAppShell);
};

}  // namespace js
}  // namespace mojo

#endif  // MOJO_SERVICES_JS_JS_APP_SHELL_H_
