// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/apps/js/mojo_bridge_module.h"

#include "gin/arguments.h"
#include "gin/converter.h"
#include "gin/object_template_builder.h"
#include "gin/per_isolate_data.h"
#include "mojo/apps/js/js_app.h"
#include "mojo/edk/js/handle.h"

namespace mojo {
namespace apps {

namespace {

gin::WrapperInfo g_wrapper_info = {gin::kEmbedderNativeGin};

}  // namespace

const char MojoInternals::kModuleName[] = "mojo/apps/js/bridge";

v8::Local<v8::Value> MojoInternals::GetModule(JSApp* js_app,
                                              v8::Isolate* isolate) {
  gin::PerIsolateData* data = gin::PerIsolateData::From(isolate);
  v8::Local<v8::ObjectTemplate> templ =
      data->GetObjectTemplate(&g_wrapper_info);

  if (templ.IsEmpty()) {
    templ = gin::ObjectTemplateBuilder(isolate)
                .SetMethod("requestorMessagePipeHandle",
                           base::Bind(&JSApp::RequestorMessagePipeHandle,
                                      base::Unretained(js_app)))
                .SetMethod("connectToApplication",
                           base::Bind(&JSApp::ConnectToApplication,
                                      base::Unretained(js_app)))
                .SetMethod("quit",
                           base::Bind(&JSApp::Quit, base::Unretained(js_app)))
                .Build();
    data->SetObjectTemplate(&g_wrapper_info, templ);
  }

  return templ->NewInstance();
}

}  // namespace apps
}  // namespace mojo
