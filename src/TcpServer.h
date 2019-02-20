/*************************************************************************
	> File Name: TcpServer.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月20日 星期三 14时35分06秒
 ************************************************************************/

#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <vector>
#include <routine.h>
#include <string>
#include "Callbacks.h"

namespace Tattoo
{

class TcpServer
{
  public:
	TcpServer(const char *ip, const int port, unsigned long long processNum, unsigned long long routineNum);
	~TcpServer();

	void start();

	void setConnectionCallback(const ConnectionCallback &Callback)
	{
		connectionCallback_ = Callback;
	}

	void setMessageCallback(const MessageCallback &Callback)
	{
		messageCallback_ = Callback;
	}

  private:
	static void *accept_routine(void *arg);
	static void *readwrite_routine(void *arg);

	//FIXME
  public:
	unsigned long long processNum_;
	unsigned long long threadNum_;
	unsigned long long routineNum_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	bool started_;
};

} // namespace Tattoo

#endif
