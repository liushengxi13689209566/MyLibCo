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

const int kPollTimeMs = 10000; // 10 s

EventLoop::EventLoop()
    : rouEnv_(get_curr_thread_env()), //  一个　eventloop  对应一个　Routine_env
      epoll_(new Epoll(this)),
      timerHeap_(new TimeHeap(this))
{
    // std::cout << "EventLoop created " << this << std::endl;
    rouEnv_->envEventLoop_ = this; //关键点
}
EventLoop::~EventLoop()
{
}
void EventLoop::loop()
{
    while (1)
    {
        activeChannels_.clear();
        int ret = epoll_->poll(kPollTimeMs, &activeChannels_);

        for (auto it = activeChannels_.begin();
             it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent(); //事件分发,记得注册时间回调（一般就是 Resume()）
        }
    }
    std::cout << "EventLoop " << this << " stop looping" << std::endl;
}
Timer *EventLoop::runAt(const Timestamp &time)
{
    return timerHeap_->addTimer(time);
}
Timer *EventLoop::runAfter(double delay)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    runAt(time);
}
void EventLoop::cancel(Timer *timer)
{
    timerHeap_->delTimer(timer);
}
void EventLoop::updateChannel(Channel *channel)
{
    epoll_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    epoll_->removeChannel(channel);
}
