
#include "InetAddress.h"
#include <arpa/inet.h>

using namespace Tattoo;

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(kInaddrAny);
    addr_.sin_port = htonl(port);
}
InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
    addr_.sin_port = htons(port);
}
