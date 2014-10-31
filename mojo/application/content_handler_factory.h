// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_APPLICATION_CONTENT_HANDLER_FACTORY_H_
#define MOJO_APPLICATION_CONTENT_HANDLER_FACTORY_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/public/interfaces/content_handler/content_handler.mojom.h"
#include "mojo/services/public/interfaces/network/url_loader.mojom.h"

namespace mojo {

class ContentHandlerFactory : public InterfaceFactory<ContentHandler> {
 public:
  class HandledApplicationHolder {
   public:
    virtual ~HandledApplicationHolder() {}
  };

  class Delegate {
   public:
    virtual ~Delegate() {}
    // Implement this method to create the Application for the given
    // content. This method will be called on a new thread. The application will
    // be run on this new thread, and the returned
    // value will be kept alive until the application ends.
    virtual scoped_ptr<HandledApplicationHolder> CreateApplication(
        ShellPtr shell,
        URLResponsePtr response) = 0;
  };

  explicit ContentHandlerFactory(Delegate* delegate);
  virtual ~ContentHandlerFactory();

 private:
  // From InterfaceFactory:
  virtual void Create(ApplicationConnection* connection,
                      InterfaceRequest<ContentHandler> request) override;

  Delegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(ContentHandlerFactory);
};

template <class A>
class HandledApplicationHolderImpl
    : public ContentHandlerFactory::HandledApplicationHolder {
 public:
  explicit HandledApplicationHolderImpl(A* value) : value_(value) {}

 private:
  scoped_ptr<A> value_;
};

template <class A>
scoped_ptr<ContentHandlerFactory::HandledApplicationHolder>
make_handled_factory_holder(A* value) {
  return make_scoped_ptr(new HandledApplicationHolderImpl<A>(value));
}

}  // namespace mojo

#endif  // MOJO_APPLICATION_CONTENT_HANDLER_FACTORY_H_
