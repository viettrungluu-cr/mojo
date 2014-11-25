// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/js/js_app_shell.h"

#include "gin/object_template_builder.h"
#include "services/js/js_app.h"

namespace mojo {
namespace js {

gin::WrapperInfo JSAppShell::kWrapperInfo = {gin::kEmbedderNativeGin};

gin::Handle<JSAppShell> JSAppShell::Create(v8::Isolate* isolate,
                                           JSApp* js_app) {
  return CreateHandle(isolate, new JSAppShell(js_app));
}

JSAppShell::JSAppShell(JSApp* js_app) : js_app_(js_app) {
}

JSAppShell::~JSAppShell() {
}

gin::ObjectTemplateBuilder JSAppShell::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<JSAppShell>::GetObjectTemplateBuilder(isolate)
      .SetMethod("connectToApplication", &JSAppShell::ConnectToApplication);
}

void JSAppShell::ConnectToApplication(
    const std::string& application_url, mojo::Handle service_provider) {
  // TODO(hansmuller): Validate arguments.
  // TODO(hansmuller): Service_provider may be a ServiceProviderProxy.
  MessagePipeHandle message_pipe_handle(service_provider.value());
  ScopedMessagePipeHandle scoped_handle(message_pipe_handle);
  js_app_->ConnectToApplication(application_url, scoped_handle.Pass());
}

}  // namespace js
}  // namespace mojo
