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
#include "Time_heap.h"

namespace Tattoo
{
// 类似于　　stCoEpoll_t *pEpoll;　结构　
//这里的网络模型架构我还是想采取　
// 一个线程一个事件循环 一个线程里面再有指定数量的协程去处理所到的事件

class TimerEpolls;

class TimerEvent : public Timer
{
  public:
  private:
	int selffd_;
	EpollCallback epollCallback;
	TimerEpolls *timee_epolls_;
	struct epoll_event env_;
};
class TimerEpolls : public Timer
{
  public:
	TimerEpolls();
	TimerEpolls(
		TimerFun timerCallback, void *arg,
		int delay, unsigned long long evsNum, int epfd,
		struct epoll_event *revents);
	~TimerEpolls();

  private:
};
class Epoll
{
  public:
	Epoll();
	~Epoll();

  private:
	static const int kInitEventListSize = 1024 * 10;
	struct epoll_event *eventList;
	int epollfd_;
};
} // namespace Tattoo
#endif
