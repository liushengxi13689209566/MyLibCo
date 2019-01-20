
#include "ServerSocket.h"
#include "Basesocket.h"

using namespace Tattoo;

ServerSocket::~ServerSocket()
{
    BaseSocket::close(sockfd_);
}
void ServerSocket::bindAddress(const InetAddress &localaddr)
{
    BaseSocket::bind(sockfd_, localaddr.getSockAddr()); // sockaddr_in *
}
void ServerSocket::listen()
{
    BaseSocket::listen(sockfd_);
}
int ServerSocket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int connfd = BaseSocket::accept(sockfd_, &addr);
    if (connfd > 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}
void ServerSocket::shutDownWrite()
{
    BaseSocket::shutDownWrite(sockfd_);
}

void ServerSocket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}
