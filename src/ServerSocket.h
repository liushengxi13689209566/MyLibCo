/*************************************************************************
	> File Name: ServerSocket.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 18时08分48秒
 ************************************************************************/

#ifndef _SERVERSOCKET_H
#define _SERVERSOCKET_H
#include "noncopyable.h"
#include "InetAddress.h"
#include <netinet/tcp.h>
#include <string.h>

struct tcp_info;

namespace Tattoo
{
class ServerSocket : noncopyable
{
  public:
	explicit ServerSocket(int sockfd) : sockfd_(sockfd) {}
	~ServerSocket();

	int getFd() const { return sockfd_; }
	// retutn true if sueccess
	// bool getTcpInfo(struct tcp_info *) const;
	// bool getTcpInfoString(char *buf, int len) const;

	void bindAddress(const InetAddress &localaddr);

	// /成功时，返回一个非负整数
	//已接受套接字的描述符
	//设置为非阻塞和close-on-exec。* peeraddr被分配。
	// /出错，返回-1，* peeraddr不变。
	void listen();
	int accept(InetAddress *peeraddr);
	void shutDownWrite();

	void setReuseAddr(bool on);

  private:
	const int sockfd_;
};
} // namespace Tattoo
#endif
