/*************************************************************************
	> File Name: InetAddress.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 21时27分30秒
 ************************************************************************/

#include "InetAddress.h"

#include "SocketsOps.h"

#include <strings.h> // bzero
#include <netinet/in.h>

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

using namespace Tattoo;

static const in_addr_t kInaddrAny = INADDR_ANY; //任何地址　

InetAddress::InetAddress(uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
	addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
	bzero(&addr_, sizeof addr_);
	sockets::fromHostPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toHostPort() const
{
	char buf[32];
	sockets::toHostPort(buf, sizeof buf, addr_);
	return buf;
}
