/*************************************************************************
	> File Name: InetAddress.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 21时27分25秒
 ************************************************************************/

#ifndef _INETADDRESS_H
#define _INETADDRESS_H
#include <string>

#include <netinet/in.h>

namespace Tattoo
{

///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress
{
  public:
	/// Constructs an endpoint with given port number.
	/// Mostly used in TcpServer listening.
	explicit InetAddress(uint16_t port);

	/// Constructs an endpoint with given ip and port.
	/// @c ip should be "1.2.3.4"
	InetAddress(const std::string &ip, uint16_t port);

	/// Constructs an endpoint with given struct @c sockaddr_in
	/// Mostly used when accepting new connections
	InetAddress(const struct sockaddr_in &addr)
		: addr_(addr)
	{
	}

	std::string toHostPort() const;

	// default copy/assignment are Okay

	const struct sockaddr_in &getSockAddrInet() const { return addr_; }
	void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

  private:
	struct sockaddr_in addr_;
};

} // namespace Tattoo

#endif
