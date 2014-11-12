// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/http_server/public/http_server_util.h"

#include "base/logging.h"
#include "mojo/public/cpp/system/data_pipe.h"

namespace mojo {

HttpResponsePtr CreateHttpResponse(uint32_t status_code,
                                   const std::string& body) {
  HttpResponsePtr response = HttpResponse::New();

  ScopedDataPipeProducerHandle producer_handle;
  uint32_t num_bytes = static_cast<uint32_t>(body.size());
  MojoCreateDataPipeOptions options = {sizeof(MojoCreateDataPipeOptions),
                                       MOJO_CREATE_DATA_PIPE_OPTIONS_FLAG_NONE,
                                       1,
                                       num_bytes};
  MojoResult result = CreateDataPipe(
      &options, &producer_handle, &response->body);
  DCHECK_EQ(MOJO_RESULT_OK, result);
  result = WriteDataRaw(
      producer_handle.get(), body.c_str(), &num_bytes,
      MOJO_WRITE_DATA_FLAG_ALL_OR_NONE);
  DCHECK_EQ(MOJO_RESULT_OK, result);
  response->status_code = status_code;
  response->content_length = num_bytes;
  return response.Pass();
}

}  // namespace mojo
