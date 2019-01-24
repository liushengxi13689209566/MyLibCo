#include "EventLoop.h"
#include "routine.cpp"

using namespace Tattoo;

EventLoop::EventLoop(MiniHeap *time_heap, EventLoopFun fun, void *arg)
    : time_heap_(time_heap),
      runInLoopFunction_(fun),
      arg_(arg)
{
}
EventLoop::~EventLoop()
{
}
void EventLoop::runInLoop()
{
    if (runInLoopFunction_)
        runInLoopFunction_(arg_);
}
void EventLoop::loop()
{
    int epollfd = get_curr_thread_env()->epoll_->epollfd_;
    EpollRes *result = get_curr_thread_env()->epoll_->result_;

    while (1)
    {
        int ret = ::epoll_wait(epollfd, result->events_, Epoll::kInitEventListSize, 1);
        for (int i = 0; i < ret; i++)
        {
            TimerEvent *timer = (TimerEvent *)result->events_[i].data.ptr;
            timer->epollCallback_(timer);
        }
        time_heap_->runOutTimeEvent();
        runInLoop();
    }
}