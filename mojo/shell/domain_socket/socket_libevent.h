// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SHELL_DOMAIN_SOCKET_SOCKET_LIBEVENT_H_
#define MOJO_SHELL_DOMAIN_SOCKET_SOCKET_LIBEVENT_H_

#include <sys/types.h>
#include <sys/socket.h>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread_checker.h"
#include "mojo/shell/domain_socket/completion_callback.h"
#include "mojo/shell/domain_socket/socket_descriptor.h"

namespace mojo {
namespace shell {

// Convenience struct for when you need a |struct sockaddr|.
struct SockaddrStorage {
  SockaddrStorage()
      : addr_len(sizeof(addr_storage)),
        addr(reinterpret_cast<struct sockaddr*>(&addr_storage)) {}
  SockaddrStorage(const SockaddrStorage& other);
  void operator=(const SockaddrStorage& other);

  struct sockaddr_storage addr_storage;
  socklen_t addr_len;
  struct sockaddr* const addr;
};

// Socket class to provide asynchronous read/write operations on top of the
// posix socket api. It supports AF_INET, AF_INET6, and AF_UNIX addresses.
class SocketLibevent : public base::MessageLoopForIO::Watcher {
 public:
  SocketLibevent();
  ~SocketLibevent() override;

  // Opens a socket and returns net::OK if |address_family| is AF_INET, AF_INET6
  // or AF_UNIX. Otherwise, it does DCHECK() and returns a net error.
  int Open(int address_family);
  // Takes ownership of |socket|.
  int AdoptConnectedSocket(SocketDescriptor socket,
                           const SockaddrStorage& peer_address);
  // Releases ownership of |socket_fd_| to caller.
  SocketDescriptor ReleaseConnectedSocket();

  int Bind(const SockaddrStorage& address);

  int Listen(int backlog);
  int Accept(scoped_ptr<SocketLibevent>* socket,
             const CompletionCallback& callback);

  // Connects socket. On non-ERR_IO_PENDING error, sets errno and returns a net
  // error code. On ERR_IO_PENDING, |callback| is called with a net error code,
  // not errno, though errno is set if connect event happens with error.
  // TODO(byungchul): Need more robust way to pass system errno.
  int Connect(const SockaddrStorage& address,
              const CompletionCallback& callback);
  bool IsConnected() const;
  bool IsConnectedAndIdle() const;

  int GetLocalAddress(SockaddrStorage* address) const;
  int GetPeerAddress(SockaddrStorage* address) const;
  void SetPeerAddress(const SockaddrStorage& address);
  // Returns true if peer address has been set regardless of socket state.
  bool HasPeerAddress() const;

  void Close();

  SocketDescriptor socket_fd() const { return socket_fd_; }

 private:
  // base::MessageLoopForIO::Watcher methods.
  void OnFileCanReadWithoutBlocking(int fd) override;
  void OnFileCanWriteWithoutBlocking(int fd) override;

  int DoAccept(scoped_ptr<SocketLibevent>* socket);
  void AcceptCompleted();

  int DoConnect();
  void ConnectCompleted();

  void StopWatchingAndCleanUp();

  SocketDescriptor socket_fd_;

  base::MessageLoopForIO::FileDescriptorWatcher accept_socket_watcher_;
  scoped_ptr<SocketLibevent>* accept_socket_;
  CompletionCallback accept_callback_;

  base::MessageLoopForIO::FileDescriptorWatcher read_socket_watcher_;
  // External callback; called when read is complete.
  CompletionCallback read_callback_;

  base::MessageLoopForIO::FileDescriptorWatcher write_socket_watcher_;
  // External callback; called when write or connect is complete.
  CompletionCallback write_callback_;

  // A connect operation is pending. In this case, |write_callback_| needs to be
  // called when connect is complete.
  bool waiting_connect_;

  scoped_ptr<SockaddrStorage> peer_address_;

  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(SocketLibevent);
};

}  // namespace shell
}  // namespace mojo

#endif  // MOJO_SHELL_DOMAIN_SOCKET_SOCKET_LIBEVENT_H_
