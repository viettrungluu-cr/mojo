// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_SERVER_SOCKET_POSIX_H_
#define MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_SERVER_SOCKET_POSIX_H_

#include <sys/types.h>

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "mojo/shell/domain_socket/completion_callback.h"
#include "mojo/shell/domain_socket/socket_descriptor.h"

namespace mojo {
namespace shell {

class SocketLibevent;

// Unix Domain Server Socket Implementation. Supports abstract namespaces on
// Linux and Android.
class UnixDomainServerSocket {
 public:
  // Credentials of a peer process connected to the socket.
  struct Credentials {
    pid_t process_id;
    uid_t user_id;
    gid_t group_id;
  };

  // Callback that returns whether the already connected client, identified by
  // its credentials, is allowed to keep the connection open. Note that
  // the socket is closed immediately in case the callback returns false.
  typedef base::Callback<bool(const Credentials&)> AuthCallback;

  UnixDomainServerSocket(const AuthCallback& auth_callack,
                         bool use_abstract_namespace);
  ~UnixDomainServerSocket();

  // Gets credentials of peer to check permissions.
  static bool GetPeerCredentials(SocketDescriptor socket_fd,
                                 Credentials* credentials);

  int ListenWithPath(const std::string& unix_domain_path, int backlog);

  // Accepts an incoming connection on |listen_socket_|, but passes back
  // a raw SocketDescriptor instead of a StreamSocket.
  int Accept(SocketDescriptor* socket_descriptor,
             const CompletionCallback& callback);

 private:
  // A callback to wrap the setting of the out-parameter to Accept().
  // This allows the internal machinery of that call to be implemented in
  // a manner that's agnostic to the caller's desired output.
  typedef base::Callback<void(scoped_ptr<SocketLibevent>)> SetterCallback;

  int DoAccept(const SetterCallback& setter_callback,
               const CompletionCallback& callback);
  void AcceptCompleted(const SetterCallback& setter_callback,
                       const CompletionCallback& callback,
                       int rv);
  bool AuthenticateAndGetStreamSocket(const SetterCallback& setter_callback);

  scoped_ptr<SocketLibevent> listen_socket_;
  const AuthCallback auth_callback_;
  const bool use_abstract_namespace_;

  scoped_ptr<SocketLibevent> accept_socket_;

  DISALLOW_COPY_AND_ASSIGN(UnixDomainServerSocket);
};

}  // namespace shell
}  // namespace mojo

#endif  // MOJO_SHELL_DOMAIN_SOCKET_UNIX_DOMAIN_SOCKET_POSIX_H_
