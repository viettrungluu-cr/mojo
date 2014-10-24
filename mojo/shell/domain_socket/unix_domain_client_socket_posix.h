// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_CLIENT_SOCKET_POSIX_H_
#define MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_CLIENT_SOCKET_POSIX_H_

#include <string>

#include "base/basictypes.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "mojo/shell/domain_socket/completion_callback.h"
#include "mojo/shell/domain_socket/socket_descriptor.h"

namespace mojo {
namespace shell {

class SocketLibevent;
struct SockaddrStorage;

// A client socket that uses unix domain socket as the transport layer.
class UnixDomainClientSocket {
 public:
  // Builds a client socket with |socket_path|. The caller should call Connect()
  // to connect to a server socket.
  UnixDomainClientSocket(const std::string& socket_path,
                         bool use_abstract_namespace);
  // Builds a client socket with socket libevent which is already connected.
  // UnixDomainServerSocket uses this after it accepts a connection.
  explicit UnixDomainClientSocket(scoped_ptr<SocketLibevent> socket);

  ~UnixDomainClientSocket();

  // Fills |address| with |socket_path| and its length. For Android or Linux
  // platform, this supports abstract namespaces.
  static bool FillAddress(const std::string& socket_path,
                          bool use_abstract_namespace,
                          SockaddrStorage* address);

  int Connect(const CompletionCallback& callback);
  void Disconnect();
  bool IsConnected() const;
  bool IsConnectedAndIdle() const;

  // Releases ownership of underlying SocketDescriptor to caller.
  // Internal state is reset so that this object can be used again.
  // Socket must be connected in order to release it.
  SocketDescriptor ReleaseConnectedSocket();

 private:
  const std::string socket_path_;
  const bool use_abstract_namespace_;
  scoped_ptr<SocketLibevent> socket_;

  DISALLOW_COPY_AND_ASSIGN(UnixDomainClientSocket);
};

}  // namespace shell
}  // namespace mojo

#endif  // MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_CLIENT_SOCKET_POSIX_H_
