/*************************************************************************
	> File Name: Time_heap.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 16时49分00秒
 ************************************************************************/

#ifndef _TIME_HEAP_H
#define _TIME_HEAP_H
#include <ctime>
#include <memory>
#include <queue>
#include "callback.h"

namespace Tattoo
{
/*定时器类*/
class Timer
{
  public:
	Timer(TimerFun timefun, void *arg, int delay)
		: timerCallback_(timefun),
		  arg_(arg)
	{
		expire_ = time(NULL) + delay;
	}
	~Timer() {}
	bool operator>(const Timer a) const //重载　＞　
	{
		return expire_ > a.expire_;
	}

	time_t expire_;			 //任务的超时事件
	TimerFun timerCallback_; // 回调函数
	void *arg_;
};

class MiniHeap
{
  public:
	using TimerPtr = std::shared_ptr<class Timer>;

	MiniHeap() {}
	~MiniHeap(){};

	/*添加定时器*/
	void AddTimer(const TimerPtr &timer)
	{
		if (!timer)
			return;
		que.push(timer);
	}
	void AddTimer(TimerFun timefun, void *arg, int delay)
	{
		que.push(std::make_shared<Timer>(timefun, arg, delay));
	}
	/*删除定时器（肯定是优先队列的第一个元素）想想为什么？？*/
	void DelTimer(Timer *timer)
	{
		if (!timer)
			return;
		que.pop();
	}
	time_t TopTime()
	{
		if (que.size() == 0)
			return 5;
		auto curr = time(NULL);
		return que.top()->expire_ - curr;
	}
	/*SIGALRM 信号被触发，就在信号处理函数中执行一次tick函数，以处理到期的任务*/
	void runOutTimeEvent()
	{
		if (que.size() == 0)
			return;
		// std::cout << "time tick " << std::endl;
		time_t curr = time(NULL);
		/*从头处理到期的定时器，直到遇到一个尚未到期的定时器*/
		while (!que.empty())
		{
			auto tt = que.top();
			if (curr < tt->expire_)
			{
				break;
			}
			//堆顶元素时间到期　
			else
			{
				if (tt->timerCallback_)
				{
					tt->timerCallback_(tt->arg_);
					/*执行完就把他从队列中删除*/
					que.pop();
				}
			}
		}
	}

  private:
	std::priority_queue<TimerPtr, std::vector<TimerPtr>, std::greater<TimerPtr>> que;
};
} // namespace Tattoo
#endif
