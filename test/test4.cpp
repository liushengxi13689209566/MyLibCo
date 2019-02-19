/*************************************************************************
	> File Name: test4.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 20时58分53秒
 ************************************************************************/

// copied from Tattoo/net/tests/TimerQueue_unittest.cc

#include "EventLoop.h"
#include <stdlib.h>

#include <stdio.h>

#if (0)
int cnt = 0;
EventLoop *g_loop;

void printTid()
{
	printf("now %s\n", Timestamp::now().toString().c_str());
}

void print(const char *msg)
{
	printf("msg %s %s\n", Timestamp::now().toString().c_str(), msg);
	if (++cnt == 20)
	{
		exit(0);
	}
}

int main()
{
	printTid();
	EventLoop loop;
	g_loop = &loop;

	print("main");
	loop.runAfter(1, std::bind(print, "once1"));
	loop.runAfter(1.5, std::bind(print, "once1.5"));
	loop.runAfter(2.5, std::bind(print, "once2.5"));
	loop.runAfter(3.5, std::bind(print, "once3.5"));

	loop.loop();

	print("main loop exits");
}
#endif
#if (1)
#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include "Acceptor.cpp"
#include "EventLoop.cpp"
#include "InetAddress.cpp"
#include "SocketsOps.cpp"
#include "Socket.cpp"
#include "Channel.cpp"
#include "Timestamp.cpp"
#include "MiniHeap.cpp"
#include "Epoll.cpp"

using namespace Tattoo;

// muduo  echo  server

void onConnection(const TcpConnectionPtr &conn)
{
	if (conn->connected())
	{
		printf("onConnection(): new connection [%s] from %s\n",
			   conn->name().c_str(),
			   conn->peerAddress().toHostPort().c_str());
	}
	else
	{
		printf("onConnection(): connection [%s] is down\n",
			   conn->name().c_str());
	}
}

void onMessage(const TcpConnectionPtr &conn,
			   Buffer *buf,
			   Timestamp receiveTime)
{
	printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
		   buf->readableBytes(),
		   conn->name().c_str(),
		   receiveTime.toFormattedString().c_str());

	conn->send(buf->retrieveAsString());
}

int main(void)
{
	printf("main(): pid = %d\n", getpid());

	InetAddress listenAddr(9981);
	EventLoop loop;

	TcpServer server(&loop, listenAddr);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.start();

	loop.loop();
}
