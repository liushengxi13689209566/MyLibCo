/*************************************************************************
	> File Name: Epoll.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 22时07分58秒
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H
#include <vector>
#include <ctime>
#include <map>
#include "Callbacks.h"
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Channel.h"

namespace Tattoo
{
class Epoll
{
  public:
	typedef std::vector<Channel *> ChannelList;

	Epoll(EventLoop *loop);
	~Epoll();

	int poll(int timeoutMs, ChannelList *activeChannels);

	void updateChannel(Channel *channel);

  private:
	void fillActiveChannels(int numEvents,
							ChannelList *activeChannels) const;

	void update(int operation, Channel *channel);
	static const char *operationToString(int op);

	typedef std::vector<struct epoll_event> EpollEventList;
	typedef std::map<int, Channel *> ChannelMap; //key是文件描述符，value是Channel *

	static const int kInitEventListSize = 1024;
	int epollfd_;

	EventLoop *owerLoop_;
	EpollEventList events_;
	ChannelMap channels_;
};
} // namespace Tattoo
#endif
