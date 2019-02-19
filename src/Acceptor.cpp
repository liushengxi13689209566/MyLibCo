/*************************************************************************
	> File Name: Acceptor.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 21时21分18秒
 ************************************************************************/

#include "Acceptor.h"

#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

using namespace Tattoo;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
	: loop_(loop),
	  acceptSocket_(sockets::createNonblockingOrDie()),
	  acceptChannel_(loop, acceptSocket_.fd()),
	  listenning_(false)
{
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback(
		std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
	listenning_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
	InetAddress peerAddr(0);
	//FIXME loop until no more
	int connfd = acceptSocket_.accept(&peerAddr);
	if (connfd >= 0)
	{
		if (newConnectionCallback_)
		{
			newConnectionCallback_(connfd, peerAddr);
		}
		else
		{
			sockets::close(connfd);
		}
	}
}
