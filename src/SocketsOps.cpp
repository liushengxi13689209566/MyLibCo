// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>   // snprintf
#include <strings.h> // bzero
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

using namespace Tattoo;

namespace
{

typedef struct sockaddr SA;

const SA *sockaddr_cast(const struct sockaddr_in *addr)
{
  return static_cast<const SA *>(reinterpret_cast<const void *>(addr));
}

SA *sockaddr_cast(struct sockaddr_in *addr)
{
  return static_cast<SA *>(reinterpret_cast<void *>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd)
{
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check
}

} // namespace

//创建非阻塞 socket
int sockets::createNonblockingOrDie()
{
  int sockfd = ::socket(AF_INET,
                        SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
  if (sockfd < 0)
  {
    std::cout << "sockets::createNonblockingOrDie" << std::endl;
  }
  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr)
{
  int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
  if (ret < 0)
  {
    std::cout << "sockets::bindOrDie" << std::endl;
  }
}

void sockets::listenOrDie(int sockfd)
{
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0)
  {
    std::cout << "sockets::listenOrDie" << std::endl;
  }
}

int sockets::accept(int sockfd, struct sockaddr_in *addr)
{
  socklen_t addrlen = sizeof *addr;
#if VALGRIND
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0)
  {
    int savedErrno = errno;
    std::cout << "Socket::accept" << std::endl;
    switch (savedErrno)
    {
    case EAGAIN:
    case ECONNABORTED:
    case EINTR:
    case EPROTO: // ???
    case EPERM:
    case EMFILE: // per-process lmit of open file desctiptor ???
      // expected errors
      errno = savedErrno;
      break;
    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
      // unexpected errors
      std::cout << "unexpected error of ::accept " << savedErrno << std::endl;
      break;
    default:
      std::cout << "unknown error of ::accept " << savedErrno << std::endl;
      break;
    }
  }
  return connfd;
}

void sockets::close(int sockfd)
{
  if (::close(sockfd) < 0)
  {
    std::cout << "sockets::close" << std::endl;
  }
}

void sockets::toHostPort(char *buf, size_t size,
                         const struct sockaddr_in &addr)
{
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void sockets::fromHostPort(const char *ip, uint16_t port,
                           struct sockaddr_in *addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
    std::cout << "sockets::fromHostPort" << std::endl;
  }
}
