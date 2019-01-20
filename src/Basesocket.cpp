
#include "Basesocket.h"
#include <errno.h>
#include <fcntl.h>

using namespace Tattoo;
namespace
{
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

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

    (void)ret;
}
} // namespace

void BaseSocket::close(int sockfd)
{
    if (::close(sockfd) < 0)
        std::cout << "BaseSocket::close   failed" << std::endl;
}
void BaseSocket::bind(int sockfd, struct sockaddr_in addr)
{
    int ret = ::bind(sockfd, (SA *)&addr, static_cast<socklen_t>(sizeof(addr)));
    if (ret < 0)
        std::cout << "BaseSocket::bind   failed" << std::endl;
}

int BaseSocket::listen(int sockfd)
{
    if (::listen(sockfd, 1024) < 0)
        std::cout << "BaseSocket::listen   failed" << std::endl;
}
int BaseSocket::accept(int sockfd, SA_IN *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept(sockfd, reinterpret_cast<SA *>(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
    if (connfd < 0)
    {
        int savedErrno = errno;
        std::cout << "BaseSocket::accept   failed" << std::endl;
    }
    return connfd;
}
void BaseSocket::shutDownWrite(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0)
        std::cout << "BaseSocket::shutDownWrite   failed" << std::endl;
}