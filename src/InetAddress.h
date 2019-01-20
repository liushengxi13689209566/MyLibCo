/*************************************************************************
	> File Name: InetAddress.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 21时24分18秒
 ************************************************************************/

#ifndef _INETADDRESS_H
#define _INETADDRESS_H
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

namespace Tattoo
{
class InetAddress
{

  public:
	InetAddress(uint16_t port);

	InetAddress(const std::string &ip, uint16_t port);

	InetAddress(const struct sockaddr_in &addr)
		: addr_(addr) {}

	struct sockaddr_in getSockAddr() const { return addr_; }
	void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; };

	~InetAddress();

  private:
	struct sockaddr_in addr_;
};

} // namespace Tattoo
#endif
