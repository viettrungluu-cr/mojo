// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/application_manager/application_loader.h"

#include "base/bind.h"
#include "base/logging.h"

namespace mojo {

namespace {

void NotReached(const GURL& url,
                ScopedMessagePipeHandle shell_handle,
                URLResponsePtr response) {
  NOTREACHED();
}

}  // namespace

ApplicationLoader::LoadCallback ApplicationLoader::SimpleLoadCallback() {
  return base::Bind(&NotReached);
}

}  // namespace mojo
