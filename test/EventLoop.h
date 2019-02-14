/*************************************************************************
	> File Name: EventLoop.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 09时45分18秒
 ************************************************************************/

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H

#include "Time_heap.h"
// #include "routine.cpp"

// 这是Reactor模式的核心，每个Reactor线程内部调用一个EventLoop，
// 内部不停的进行poll或者epoll_wait调用，然后根据fd的返回事件，
// 调用fd对应Channel的相应回调函数./test
namespace Tattoo
{
class EventLoop
{
  public:
	EventLoop(MiniHeap *time_heap, EventLoopFun fun, void *arg)
		: time_heap_(time_heap),
		  runInLoopFunction_(fun),
		  arg_(arg)
	{
	}
	~EventLoop()
	{
	}
	void loop()
	{
		int epollfd = get_curr_thread_env()->epoll_->epollfd_;
		EpollRes *result = get_curr_thread_env()->epoll_->result_;
		while (1)
		{
			int ret = ::epoll_wait(epollfd, result->events_, Epoll::kInitEventListSize, -1);
			std::cout << "epoll_wait " << std::endl;

			for (int i = 0; i < ret; i++)
			{
				TimerEvent *timer = (TimerEvent *)result->events_[i].data.ptr;
				timer->epollCallback_(timer);
			}
			time_heap_->runOutTimeEvent();
			runInLoop(); //类似 muduo
		}
	}
	void runInLoop()
	{
		if (runInLoopFunction_)
			runInLoopFunction_(arg_);
	}

	EventLoopFun runInLoopFunction_;
	void *arg_;

	MiniHeap *time_heap_;
};
} // namespace Tattoo

#endif
