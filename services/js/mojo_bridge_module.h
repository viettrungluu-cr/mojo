// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_JS_MOJO_MODULE_H_
#define MOJO_SERVICES_JS_MOJO_MODULE_H_

#include "gin/gin_export.h"
#include "v8/include/v8.h"

namespace mojo {
namespace js {

class JSApp;

// The JavaScript "services/public/js/mojo" module depends on this
// built-in module. It provides the bridge between the JSApp class and
// JavaScript.

class MojoInternals {
 public:
  static const char kModuleName[];
  static v8::Local<v8::Value> GetModule(JSApp* js_app, v8::Isolate* isolate);
};

}  // namespace js
}  // namespace mojo

#endif  // MOJO_SERVICES_JS_MOJO_MODULE_H_
