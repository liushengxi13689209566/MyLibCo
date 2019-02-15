/*************************************************************************
	> File Name: Acceptor.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 21时21分13秒
 ************************************************************************/

#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H
#include <functional>

#include "Channel.h"
#include "Socket.h"

namespace Tattoo
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor 
{
 public:
  typedef std::function<void (int sockfd,
                                const InetAddress&)> NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr);

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
};

}
#endif
