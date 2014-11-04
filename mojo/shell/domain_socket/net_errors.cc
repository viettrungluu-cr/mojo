// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/domain_socket/net_errors.h"

#include <errno.h>
#include <stdlib.h>
#if defined(OS_POSIX)
#include <unistd.h>
#endif
#if defined(OS_WIN)
#include <winsock2.h>
#define EDQUOT WSAEDQUOT
#define EHOSTDOWN WSAEHOSTDOWN
#define EUSERS WSAEUSERS
#endif

#include <string>

#include "base/basictypes.h"
#include "base/files/file.h"
#include "base/logging.h"

namespace mojo {
namespace shell {
namespace net {

std::string ErrorToString(int error) {
  if (error == 0)
    return "OK";

  const char* error_string;
  switch (error) {
#define NET_ERROR(label, value) \
  case ERR_ ## label: \
    error_string = # label; \
    break;
#include "mojo/shell/domain_socket/net_error_list.h"
#undef NET_ERROR
  default:
    NOTREACHED();
    error_string = "<unknown>";
  }
  return std::string("ERR_") + error_string;
}

Error FileErrorToNetError(base::File::Error file_error) {
  switch (file_error) {
    case base::File::FILE_OK:
      return net::OK;
    case base::File::FILE_ERROR_ACCESS_DENIED:
      return net::ERR_ACCESS_DENIED;
    case base::File::FILE_ERROR_NOT_FOUND:
      return net::ERR_FILE_NOT_FOUND;
    default:
      return net::ERR_FAILED;
  }
}

Error MapSystemError(int os_error) {
  if (os_error != 0)
    DVLOG(2) << "Error " << os_error;

  // There are numerous posix error codes, but these are the ones we thus far
  // find interesting.
  switch (os_error) {
    case EAGAIN:
#if EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
      return ERR_IO_PENDING;
    case EACCES:
      return ERR_ACCESS_DENIED;
    case ENETDOWN:
      return ERR_INTERNET_DISCONNECTED;
    case ETIMEDOUT:
      return ERR_TIMED_OUT;
    case ECONNRESET:
    case ENETRESET:  // Related to keep-alive.
    case EPIPE:
      return ERR_CONNECTION_RESET;
    case ECONNABORTED:
      return ERR_CONNECTION_ABORTED;
    case ECONNREFUSED:
      return ERR_CONNECTION_REFUSED;
    case EHOSTUNREACH:
    case EHOSTDOWN:
    case ENETUNREACH:
    case EAFNOSUPPORT:
      return ERR_ADDRESS_UNREACHABLE;
    case EADDRNOTAVAIL:
      return ERR_ADDRESS_INVALID;
    case EMSGSIZE:
      return ERR_MSG_TOO_BIG;
    case ENOTCONN:
      return ERR_SOCKET_NOT_CONNECTED;
    case EISCONN:
      return ERR_SOCKET_IS_CONNECTED;
    case EINVAL:
      return ERR_INVALID_ARGUMENT;
    case EADDRINUSE:
      return ERR_ADDRESS_IN_USE;
    case E2BIG:  // Argument list too long.
      return ERR_INVALID_ARGUMENT;
    case EBADF:  // Bad file descriptor.
      return ERR_INVALID_HANDLE;
    case EBUSY:  // Device or resource busy.
      return ERR_INSUFFICIENT_RESOURCES;
    case ECANCELED:  // Operation canceled.
      return ERR_ABORTED;
    case EDEADLK:  // Resource deadlock avoided.
      return ERR_INSUFFICIENT_RESOURCES;
    case EDQUOT:  // Disk quota exceeded.
      return ERR_FILE_NO_SPACE;
    case EEXIST:  // File exists.
      return ERR_FILE_EXISTS;
    case EFAULT:  // Bad address.
      return ERR_INVALID_ARGUMENT;
    case EFBIG:  // File too large.
      return ERR_FILE_TOO_BIG;
    case EISDIR:  // Operation not allowed for a directory.
      return ERR_ACCESS_DENIED;
    case ENAMETOOLONG:  // Filename too long.
      return ERR_FILE_PATH_TOO_LONG;
    case ENFILE:  // Too many open files in system.
      return ERR_INSUFFICIENT_RESOURCES;
    case ENOBUFS:  // No buffer space available.
      return ERR_OUT_OF_MEMORY;
    case ENODEV:  // No such device.
      return ERR_INVALID_ARGUMENT;
    case ENOENT:  // No such file or directory.
      return ERR_FILE_NOT_FOUND;
    case ENOLCK:  // No locks available.
      return ERR_INSUFFICIENT_RESOURCES;
    case ENOMEM:  // Not enough space.
      return ERR_OUT_OF_MEMORY;
    case ENOSPC:  // No space left on device.
      return ERR_FILE_NO_SPACE;
    case ENOSYS:  // Function not implemented.
      return ERR_NOT_IMPLEMENTED;
    case ENOTDIR:  // Not a directory.
      return ERR_FILE_NOT_FOUND;
    case ENOTSUP:  // Operation not supported.
      return ERR_NOT_IMPLEMENTED;
    case EPERM:  // Operation not permitted.
      return ERR_ACCESS_DENIED;
    case EROFS:  // Read-only file system.
      return ERR_ACCESS_DENIED;
    case ETXTBSY:  // Text file busy.
      return ERR_ACCESS_DENIED;
    case EUSERS:  // Too many users.
      return ERR_INSUFFICIENT_RESOURCES;
    case EMFILE:  // Too many open files.
      return ERR_INSUFFICIENT_RESOURCES;

    case 0:
      return OK;
    default:
      LOG(WARNING) << "Unknown error " << os_error
                   << " mapped to net::ERR_FAILED";
      return ERR_FAILED;
  }
}

}  // namespace net
}  // namespace shell
}  // namespace mojo
