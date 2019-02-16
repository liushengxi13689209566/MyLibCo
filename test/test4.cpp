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
Tattoo::EventLoop *g_loop;

void printTid()
{
	printf("now %s\n", Tattoo::Timestamp::now().toString().c_str());
}

void print(const char *msg)
{
	printf("msg %s %s\n", Tattoo::Timestamp::now().toString().c_str(), msg);
	if (++cnt == 20)
	{
		exit(0);
	}
}

int main()
{
	printTid();
	Tattoo::EventLoop loop;
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
#include <stdio.h>
#include <unistd.h>

using namespace Tattoo;

void newConnection(int sockfd, const Tattoo::InetAddress &peerAddr)
{
	printf("newConnection(): accepted a new connection from %s\n",
		   peerAddr.toHostPort().c_str());
	::write(sockfd, "How are you?\n", 13);
	Tattoo::sockets::close(sockfd);
}

int main()
{
	printf("main(): pid = %d\n", getpid());

	Tattoo::InetAddress listenAddr(9981);
	Tattoo::EventLoop loop;

	Tattoo::Acceptor acceptor(&loop, listenAddr);
	acceptor.setNewConnectionCallback(newConnection);
	acceptor.listen();

	loop.loop();
}

#endif
