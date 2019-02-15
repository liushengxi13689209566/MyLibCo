/*************************************************************************
	> File Name: Socket.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 21时26分59秒
 ************************************************************************/

#ifndef _SOCKET_H
#define _SOCKET_H

namespace Tattoo
{

class InetAddress;

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delagated to OS.
class Socket
{
  public:
	explicit Socket(int sockfd)
		: sockfd_(sockfd)
	{
	}

	~Socket();

	int fd() const { return sockfd_; }

	/// abort if address in use
	void bindAddress(const InetAddress &localaddr);
	/// abort if address in use
	void listen();

	/// On success, returns a non-negative integer that is
	/// a descriptor for the accepted socket, which has been
	/// set to non-blocking and close-on-exec. *peeraddr is assigned.
	/// On error, -1 is returned, and *peeraddr is untouched.
	int accept(InetAddress *peeraddr);

	///
	/// Enable/disable SO_REUSEADDR
	///
	void setReuseAddr(bool on);

  private:
	const int sockfd_;
};

} // namespace Tattoo

#endif
