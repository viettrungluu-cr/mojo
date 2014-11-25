// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/js/js_app_runner_delegate.h"

#include "base/path_service.h"
#include "gin/modules/console.h"
#include "mojo/edk/js/core.h"
#include "mojo/edk/js/handle.h"
#include "mojo/edk/js/support.h"
#include "mojo/edk/js/threading.h"

namespace mojo {
namespace js {

namespace {

std::vector<base::FilePath> GetModuleSearchPaths() {
  std::vector<base::FilePath> search_paths(2);
  PathService::Get(base::DIR_SOURCE_ROOT, &search_paths[0]);
  PathService::Get(base::DIR_EXE, &search_paths[1]);
  search_paths[1] = search_paths[1].AppendASCII("gen");
  return search_paths;
}

} // namespace

JSAppRunnerDelegate::JSAppRunnerDelegate()
    : ModuleRunnerDelegate(GetModuleSearchPaths()) {
  AddBuiltinModule(gin::Console::kModuleName, gin::Console::GetModule);
  AddBuiltinModule(js::Core::kModuleName, js::Core::GetModule);
  AddBuiltinModule(js::Support::kModuleName, js::Support::GetModule);
  AddBuiltinModule(js::Threading::kModuleName, js::Threading::GetModule);
}

JSAppRunnerDelegate::~JSAppRunnerDelegate() {
}

void JSAppRunnerDelegate::UnhandledException(gin::ShellRunner* runner,
                                             gin::TryCatch& try_catch) {
  gin::ModuleRunnerDelegate::UnhandledException(runner, try_catch);
  LOG(ERROR) << try_catch.GetStackTrace();
}

}  // namespace mojo
} // namespace js

