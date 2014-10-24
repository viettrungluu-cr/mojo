// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/domain_socket/unix_domain_client_socket_posix.h"

#include <sys/socket.h>
#include <sys/un.h>

#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "mojo/shell/domain_socket/net_errors.h"
#include "mojo/shell/domain_socket/socket_libevent.h"

namespace mojo {
namespace shell {

UnixDomainClientSocket::UnixDomainClientSocket(const std::string& socket_path,
                                               bool use_abstract_namespace)
    : socket_path_(socket_path),
      use_abstract_namespace_(use_abstract_namespace) {
}

UnixDomainClientSocket::UnixDomainClientSocket(
    scoped_ptr<SocketLibevent> socket)
    : use_abstract_namespace_(false), socket_(socket.Pass()) {
}

UnixDomainClientSocket::~UnixDomainClientSocket() {
  Disconnect();
}

// static
bool UnixDomainClientSocket::FillAddress(const std::string& socket_path,
                                         bool use_abstract_namespace,
                                         SockaddrStorage* address) {
  struct sockaddr_un* socket_addr =
      reinterpret_cast<struct sockaddr_un*>(address->addr);
  size_t path_max = address->addr_len - offsetof(struct sockaddr_un, sun_path);
  // Non abstract namespace pathname should be null-terminated. Abstract
  // namespace pathname must start with '\0'. So, the size is always greater
  // than socket_path size by 1.
  size_t path_size = socket_path.size() + 1;
  if (path_size > path_max)
    return false;

  memset(socket_addr, 0, address->addr_len);
  socket_addr->sun_family = AF_UNIX;
  address->addr_len = path_size + offsetof(struct sockaddr_un, sun_path);
  if (!use_abstract_namespace) {
    memcpy(socket_addr->sun_path, socket_path.c_str(), socket_path.size());
    return true;
  }

#if defined(OS_ANDROID) || defined(OS_LINUX)
  // Convert the path given into abstract socket name. It must start with
  // the '\0' character, so we are adding it. |addr_len| must specify the
  // length of the structure exactly, as potentially the socket name may
  // have '\0' characters embedded (although we don't support this).
  // Note that addr.sun_path is already zero initialized.
  memcpy(socket_addr->sun_path + 1, socket_path.c_str(), socket_path.size());
  return true;
#else
  return false;
#endif
}

int UnixDomainClientSocket::Connect(const CompletionCallback& callback) {
  DCHECK(!socket_);

  if (socket_path_.empty())
    return net::ERR_ADDRESS_INVALID;

  SockaddrStorage address;
  if (!FillAddress(socket_path_, use_abstract_namespace_, &address))
    return net::ERR_ADDRESS_INVALID;

  socket_.reset(new SocketLibevent);
  int rv = socket_->Open(AF_UNIX);
  DCHECK_NE(net::ERR_IO_PENDING, rv);
  if (rv != net::OK)
    return rv;

  return socket_->Connect(address, callback);
}

void UnixDomainClientSocket::Disconnect() {
  socket_.reset();
}

bool UnixDomainClientSocket::IsConnected() const {
  return socket_ && socket_->IsConnected();
}

bool UnixDomainClientSocket::IsConnectedAndIdle() const {
  return socket_ && socket_->IsConnectedAndIdle();
}

SocketDescriptor UnixDomainClientSocket::ReleaseConnectedSocket() {
  DCHECK(socket_);
  DCHECK(socket_->IsConnected());

  SocketDescriptor socket_fd = socket_->ReleaseConnectedSocket();
  socket_.reset();
  return socket_fd;
}

}  // namespace shell
}  // namespace mojo
