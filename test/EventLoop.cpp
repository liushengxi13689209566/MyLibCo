/*************************************************************************
	> File Name: EventLoop.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月15日 星期五 16时09分50秒
 ************************************************************************/

#include <iostream>
#include "Channel.h"
#include "Epoll.h"
#include "MiniHeap.h"
#include "EventLoop.h"

using namespace Tattoo;

const int kPollTimeMs = 10000;

EventLoop::EventLoop()
	: looping_(false),
	  epoll_(new Epoll(this)),
	  timerHeap_(new TimeHeap(this))
{
	std::cout << "EventLoop created " << this << " in thread " << std::endl;
}
EventLoop::~EventLoop()
{
}

void EventLoop::loop()
{
	looping_ = true;
	while (1)
	{
		activeChannels_.clear();
		epoll_->poll(kPollTimeMs, &activeChannels_);

		for (auto it = activeChannels_.begin();
			 it != activeChannels_.end(); ++it)
		{
			(*it)->handleEvent(); //事件分发
		}
	}

	std::cout << "EventLoop " << this << " stop looping" << std::endl;
	looping_ = false;
}
void EventLoop::runAt(const Timestamp &time, const TimerCallback &cb)
{
	timerHeap_->addTimer(cb, time, 0.0);
}

void EventLoop::runAfter(double delay, const TimerCallback &cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	runAt(time, cb);
}

void EventLoop::runEvery(double interval, const TimerCallback &cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	timerHeap_->addTimer(cb, time, interval);
}
void EventLoop::runInLoop(const Functor &cb)
{
	cb();
}
void EventLoop::updateChannel(Channel *channel)
{
	epoll_->updateChannel(channel);
}
