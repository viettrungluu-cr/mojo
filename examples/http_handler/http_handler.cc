// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "services/http_server/public/http_server.mojom.h"
#include "services/http_server/public/http_server_util.h"

namespace mojo {
namespace examples {

// This is an example of a self-contained HTTP handler. It uses the HTTP Server
// service to handle the HTTP protocol details, and just contains the logic for
// handling its registered urls.
class HttpHandler : public ApplicationDelegate,
                    public HttpServerClient {
 public:
  HttpHandler() {}
  ~HttpHandler() override {}

 private:
  // ApplicationDelegate:
  void Initialize(ApplicationImpl* app) override {
    app->ConnectToService("mojo:http_server", &http_server_service_);

    http_server_service_.set_client(this);
    http_server_service_->AddHandler(
        "/test",
        base::Bind(&HttpHandler::AddHandlerCallback, base::Unretained(this)));
  }

  // HttpServerClient:
  void OnHandleRequest(
      HttpRequestPtr request,
      const Callback<void(HttpResponsePtr)>& callback) override {
    callback.Run(CreateHttpResponse(200, "Hello World\n"));
  }

  void AddHandlerCallback(bool result) {
    CHECK(result);
  }

  HttpServerServicePtr http_server_service_;

  DISALLOW_COPY_AND_ASSIGN(HttpHandler);
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::examples::HttpHandler());
  return runner.Run(shell_handle);
}
