// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include "mojo/public/c/system/main.h"

MojoResult MojoMain(MojoHandle shell_handle) {
  // Terminate the process as soon as we reach the application entrypoint. The
  // testing script will instruct the shell to load this application and
  // measure the execution time, which reflects the startup performance.
  exit(0);
  return MOJO_RESULT_OK;
}
