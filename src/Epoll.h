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
// 类似于　　stCoEpoll_t *pEpoll;　结构　
//这里的网络模型架构我还是想采取　
// 一个线程一个事件循环 一个线程里面再有指定数量的协程去处理所到的事件

class TimerEpolls;

//针对于每个epoll_event
class TimerEvent : public Timer
{
  public:
	TimerEvent() {}
	~TimerEvent() {}

  public:
	int selffd_;
	EpollCallback epollCallback_;
	TimerEpolls *timer_epolls_;
	struct epoll_event env_;
};
//epoll定时器
class TimerEpolls : public Timer
{
  public:
	TimerEpolls();
	TimerEpolls(
		TimerFun timerCallback, void *arg,
		int delay, unsigned long long evsNum, int epfd,
		struct epoll_event *revents);
	~TimerEpolls();

	int epfd_;
	struct epoll_event *revents_;
	unsigned long long evsNum_;
	int RaiseNum_;
	bool isOutTime_;
	TimerEvent *timer_event_;
};

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
