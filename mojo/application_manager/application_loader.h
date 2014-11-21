// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_APPLICATION_MANAGER_APPLICATION_LOADER_H_
#define MOJO_APPLICATION_MANAGER_APPLICATION_LOADER_H_

#include "base/callback.h"
#include "mojo/application_manager/application_manager_export.h"
#include "mojo/public/cpp/system/core.h"
#include "mojo/services/public/interfaces/network/url_loader.mojom.h"
#include "url/gurl.h"

namespace mojo {

class ApplicationManager;

// Interface to allowing loading behavior to be established for schemes,
// specific urls or as the default.
// A ApplicationLoader is responsible to using whatever mechanism is appropriate
// to load the application at url.
// The handle to the shell is passed to that application so it can bind it to
// a Shell instance. This will give the Application a way to connect to other
// apps and services.
class MOJO_APPLICATION_MANAGER_EXPORT ApplicationLoader {
 public:
  typedef base::Callback<
      void(const GURL&, ScopedMessagePipeHandle, URLResponsePtr)> LoadCallback;
  virtual ~ApplicationLoader() {}

  // Returns a callback that will should never be called.
  static LoadCallback SimpleLoadCallback();

  // Load the application named |url|. Applications can be loaded two ways:
  //
  // 1. |url| can refer directly to a Mojo application. In this case,
  //    shell_handle should be used to implement the mojo.Application interface.
  //
  // 2. |url| can refer to some content that can be handled by some other Mojo
  //    application. In this case, call callbacks and specify the URL of the
  //    application that should handle the content.  The specified application
  //    must implement the mojo.ContentHandler interface.
  virtual void Load(ApplicationManager* application_manager,
                    const GURL& url,
                    ScopedMessagePipeHandle shell_handle,
                    LoadCallback callback) = 0;

  // Called when the Application exits.
  virtual void OnApplicationError(ApplicationManager* manager,
                                  const GURL& url) = 0;

 protected:
  ApplicationLoader() {}
};

}  // namespace mojo

#endif  // MOJO_APPLICATION_MANAGER_APPLICATION_LOADER_H_
