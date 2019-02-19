/*************************************************************************
	> File Name: Time_heap.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 16时49分00秒
 ************************************************************************/

#ifndef _MINIHEAP_H
#define _MINIHEAP_H
#include <map>
#include <vector>
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"

namespace Tattoo
{
class EventLoop;

/*定时器类*/
class Timer
{
public:
	Timer(const TimerCallback &cb, Timestamp when, int interval)
			: callback_(cb),
				expire_(when),
				interval_(interval),
				repeat_(interval > 0.0)
	{
	}
	Timestamp expiration() const { return expire_; }
	bool repeat() const { return repeat_; }

	void restart(Timestamp now)
	{
		if (repeat_)
		{
			expire_ = addTime(now, interval_);
		}
		else
		{
			expire_ = Timestamp::invalid();
		}
	}

	void run() const
	{
		callback_();
	}

private:
	Timestamp expire_;						 //任务的超时事件
	const TimerCallback callback_; // 回调函数
	const double interval_;
	const bool repeat_;
};

class TimeHeap
{
public:
	TimeHeap(EventLoop *loop);
	~TimeHeap();

	void addTimer(const TimerCallback &cb, Timestamp when, double interval);
	//FIXME:
	// void cancel(TimerId timerId);

private:
	typedef std::pair<Timestamp, Timer *> Entry;
	typedef std::multimap<Timestamp, Timer *> TimerMap;

	void addTimerInLoop(Timer *timer);
	// called when timerfd alarms
	void handleRead();
	// move out all expired timers
	std::vector<Entry> getExpired(Timestamp now);
	/* 重置超时的定时器 */
	void reset(const std::vector<Entry> &expired, Timestamp now);

	bool insert(Timer *timer);

	EventLoop *loop_;
	const int timerfd_;

	Channel timerfdChannel_;
	// Timer list sorted by expiration
	TimerMap timers_;
};
} // namespace Tattoo
#endif
