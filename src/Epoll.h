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
#include "callback.h"
#include <sys/epoll.h>

namespace Tattoo
{
class Epoll
{
public:
	Epoll(EventLoop *loop)
	{
		epollfd_ = ::epoll_create(kInitEventListSize);
		result_ = new EpollRes(kInitEventListSize);
	}
	~Epoll() {}

	int poll(int timeoutMs, ChannelList *activeChannels);

private:
	typedef std::vector<struct epoll_event> EpollEventList;
	typedef std::map<int, Channel *> ChannelMap; //key是文件描述符，value是Channel *

	static const int kInitEventListSize = 1024 * 10;
	int epollfd_;

	EventLoop *ownreLoop_;
	EpollEventList events_;
	ChannelMap channels_;
};
} // namespace Tattoo
#endif
