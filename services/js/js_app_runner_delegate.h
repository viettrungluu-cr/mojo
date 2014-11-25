// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_JS_JS_APP_RUNNER_DELEGATE_H_
#define SERVICES_JS_JS_APP_RUNNER_DELEGATE_H_

#include "gin/modules/module_runner_delegate.h"
#include "gin/shell_runner.h"
#include "gin/try_catch.h"

namespace mojo {
namespace js {

class JSAppRunnerDelegate : public gin::ModuleRunnerDelegate {
 public:
  JSAppRunnerDelegate();
  ~JSAppRunnerDelegate() override;

  // From ModuleRunnerDelegate:
  void UnhandledException(gin::ShellRunner* runner,
                          gin::TryCatch& try_catch) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(JSAppRunnerDelegate);
};

}  // namespace js
}  // namespace mojo

#endif  // SERVICES_JS_JS_APP_RUNNER_DELEGATE_H_
