/*************************************************************************
	> File Name: Time_heap.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 16时49分00秒
 ************************************************************************/

#include "Time_heap.h"

#include <sys/timerfd.h>
#include <iostream>
#include <unistd.h>
#include <functional>
#include <assert.h>
#include <cstring>

namespace Tattoo
{
namespace detail
{
int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
								   TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
	{
		std::cout << "Failed in timerfd_create" << std::endl;
	}
	return timerfd;
}
struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
		microseconds / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(
		(microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
	return ts;
}
void readTimerfd(int timerfd, Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
	std::cout << "TimerQueue::handleRead() " << howmany << " at " << now.toString() << std::endl;
	if (n != sizeof howmany)
	{
		std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
	}
}
void resetTimerfd(int timerfd, Timestamp expiration)
{
	// wake up loop by timerfd_settime()
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret)
	{
		std::cout << "timerfd_settime()" << std::endl;
	}
}
} // namespace detail
} // namespace Tattoo

using namespace Tattoo;
using namespace Tattoo::detail;

TimeHeap::TimeHeap(EventLoop *loop)
	: loop_(loop),
	  timerfd_(createTimerfd()),
	  timerfdChannel_(loop, timerfd_),
	  timers_()
{
	timerfdChannel_.setReadCallback(
		std::bind(&TimeHeap::handleRead, this));
	// we are always reading the timerfd, we disarm it with timerfd_settime.
	timerfdChannel_.enableReading();
}
TimeHeap::~TimeHeap()
{
	::close(timerfd_);
	// do not remove channel, since we're in EventLoop::dtor();
	for (auto it = timers_.begin();
		 it != timers_.end(); ++it)
	{
		delete it->second;
	}
}
/* 添加一个定时器 */
void TimeHeap::addTimer(const TimerCallback &cb,Timestamp when, int interval)
{
	Timer *timer = new Timer(cb,when, interval);
	loop_->runInLoop(
		std::bind(&TimeHeap::addTimerInLoop, this, timer));
	return;
}
//添加一个定时任务
void TimeHeap::addTimerInLoop(Timer *timer)
{
	bool earliestChanged = insert(timer);
	if (earliestChanged)
	{
		resetTimerfd(timerfd_, timer->expiration());
	}
}
//timerfd 可读 的回调
void TimeHeap::handleRead()
{
	Timestamp now(Timestamp::now());
	readTimerfd(timerfd_, now);

	std::vector<Entry> expired = getExpired(now);

	// safe to callback outside critical section
	for (std::vector<Entry>::iterator it = expired.begin();
		 it != expired.end(); ++it)
	{
		it->second->run();
	}

	reset(expired, now);
}

//获取所有超时的定时器
std::vector<TimeHeap::Entry> TimeHeap::getExpired(Timestamp now)
{
	std::vector<Entry> expired;
	auto it = timers_.lower_bound(now);
	assert(it == timers_.end() || now < it->first);
	std::copy(timers_.begin(), it, back_inserter(expired));
	timers_.erase(timers_.begin(), it);

	return expired;
}
void TimeHeap::reset(const std::vector<Entry> &expired, Timestamp now)
{
	Timestamp nextExpire;

	for (std::vector<Entry>::const_iterator it = expired.begin();
		 it != expired.end(); ++it)
	{
		if (it->second->repeat())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
			// FIXME move to a free list
			delete it->second;
		}
	}

	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}

	if (nextExpire.valid())
	{
		resetTimerfd(timerfd_, nextExpire);
	}
}

bool TimeHeap::insert(Timer *timer)
{
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	auto it = timers_.begin();
	if (it == timers_.end() || when < it->first)
	{
		earliestChanged = true;
	}
	timers_.insert(std::make_pair(when, timer));
	return earliestChanged;
}