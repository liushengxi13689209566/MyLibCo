/*************************************************************************
	> File Name: TcpServer.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月19日 星期二 09时30分22秒
 ************************************************************************/

#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <map>
#include "Callbacks.h"
#include "TcpConnection.h"

namespace Tattoo
{

class Acceptor;
class EventLoop;

class TcpServer
{
  public:
	TcpServer(EventLoop *loop, const InetAddress &listenAddr);
	~TcpServer(); // force out-line dtor, for scoped_ptr members.

	/// Starts the server if it's not listenning.
	///
	/// It's harmless to call it multiple times.
	/// Thread safe.
	void start();

	/// Set connection callback.
	/// Not thread safe.
	void setConnectionCallback(const ConnectionCallback &cb)
	{
		connectionCallback_ = cb;
	}

	/// Set message callback.
	/// Not thread safe.
	void setMessageCallback(const MessageCallback &cb)
	{
		messageCallback_ = cb;
	}

  private:
	/// Not thread safe, but in loop
	void newConnection(int sockfd, const InetAddress &peerAddr);
	void removeConnection(const TcpConnectionPtr &conn);

	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

	EventLoop *loop_; // the acceptor loop
	const std::string name_;
	Acceptor *acceptor_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	bool started_;
	int nextConnId_; // always in loop thread
	ConnectionMap connections_;
};

} // namespace Tattoo

#endif