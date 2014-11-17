// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EXAMPLES_TEST_EXAMPLE_SERVICE_IMPL_H_
#define MOJO_EXAMPLES_TEST_EXAMPLE_SERVICE_IMPL_H_

#include "examples/apptest/example_service.mojom.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/cpp/system/macros.h"

namespace mojo {

class ApplicationConnection;

class ExampleServiceImpl : public ExampleService {
 public:
  explicit ExampleServiceImpl(InterfaceRequest<ExampleService> request);
  ~ExampleServiceImpl() override;

 private:
  // ExampleService overrides.
  void Ping(uint16_t ping_value) override;
  void RunCallback(const Callback<void()>& callback) override;

  // ExampleServiceImpl is strongly bound to the pipe.
  StrongBinding<ExampleService> binding_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(ExampleServiceImpl);
};

}  // namespace mojo

#endif  // MOJO_EXAMPLES_TEST_EXAMPLE_SERVICE_IMPL_H_
