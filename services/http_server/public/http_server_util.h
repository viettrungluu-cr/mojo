// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_HTTP_SERVER_PUBLIC_HTTP_SERVER_UTIL_H_
#define SERVICES_HTTP_SERVER_PUBLIC_HTTP_SERVER_UTIL_H_

#include "services/http_server/public/http_response.mojom.h"

namespace mojo {

// Helper method to create an HttpResponse given the status code and body.
HttpResponsePtr CreateHttpResponse(uint32_t status_code,
                                   const std::string& body);
}  // namespace mojo

#endif  // SERVICES_HTTP_SERVER_PUBLIC_HTTP_SERVER_UTIL_H_
