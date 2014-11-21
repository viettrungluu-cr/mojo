// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_vector.h"
#include "examples/indirect_service/indirect_service_demo.mojom.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_runner.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/bindings/binding.h"

namespace mojo {
namespace examples {

class IndirectIntegerServiceImpl :
    public InterfaceImpl<IndirectIntegerService>, public IntegerService {
 public:
  IndirectIntegerServiceImpl() {}
  ~IndirectIntegerServiceImpl() override {}

  // IndirectIntegerService

  void Set(IntegerServicePtr service) override {
    integer_service_ = service.Pass();
  }

  void Get(InterfaceRequest<IntegerService> service) override {
    bindings_.push_back(new Binding<IntegerService>(this, service.Pass()));
  }

  // IntegerService

  void Increment(const Callback<void(int32_t)>& callback) override {
    if (integer_service_.get())
      integer_service_->Increment(callback);
  }

private:
  IntegerServicePtr integer_service_;
  ScopedVector<Binding<IntegerService>> bindings_;
};

class IndirectIntegerServiceAppDelegate : public ApplicationDelegate {
 public:
  bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(&indirect_integer_service_factory_);
    return true;
  }

 private:
  InterfaceFactoryImpl<IndirectIntegerServiceImpl>
      indirect_integer_service_factory_;
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunner runner(
      new mojo::examples::IndirectIntegerServiceAppDelegate);
  return runner.Run(shell_handle);
}

