
#include "Epoll.h"
// #include "Log.h"
#include "routine.h"
#include <sys/epoll.h>
#include <assert.h>
using namespace Tattoo;

static void *timerCallback(Routine_t *rou)
{
    rou->Resume();
    return NULL;
}
//epoll 产生事件时，唤醒 TimerEpolls
static void *epollCallback(void *arg)
{
    // LOG("Tattoo");
    TimerEvent *timer_event = (TimerEvent *)arg;
    TimerEpolls *timer_epolls = timer_event->timer_epolls_;

    timer_epolls->revents_[timer_epolls->RaiseNum_] = timer_event->env_;
    timer_epolls->revents_[timer_epolls->RaiseNum_].data.fd = timer_event->selffd_;
    timer_epolls->RaiseNum_++;
    // LOG_DEBUG("Epoll come");
    if (!timer_epolls->isOutTime_)
    {
        timer_epolls->isOutTime_ = 1;
        TimerEpolls *timerNew = new TimerEpolls();
        memcpy(timerNew, timer_epolls, sizeof(TimerEpolls));
        timerNew->expire_ = time(NULL) + 1;

        get_curr_thread_env()->time_heap_->AddTimer(timerNew);

        std::cout << "Timer_Epolls will outtime!" << std::endl;

        get_curr_thread_env()->time_heap_->DelTimer(timer_epolls);
    }
}
TimerEpolls::TimerEpolls(
    TimerFun timerCallback, void *arg,
    int delay, unsigned long long evsNum, int epfd,
    struct epoll_event *revents)
    : Timer(timerCallback, arg, delay),
      epfd_(epfd),
      evsNum_(evsNum),
      RaiseNum_(0),
      revents_(revents)
{
    std::cout << __FUNCTION__ << " : " << __LINE__ << "TimerEpolls construct ----------------------------" << std::endl;

    timer_event_ = new TimerEvent[evsNum_];
}
TimerEpolls::TimerEpolls()
{
    std::cout << __FUNCTION__ << " : " << __LINE__ << "TimerEpolls construct ----------------------------" << std::endl;
}

TimerEpolls::~TimerEpolls()
{
    delete timer_event_;
}

int Epoll::addEpoll(struct epoll_event *evs, unsigned long long evNum,
                    struct epoll_event *revents, int timeout,
                    unsigned long long maxNum)
{
    std::cout << __FUNCTION__ << " : " << __LINE__ << " addEpoll start----------------------------" << std::endl;

    TimerEpolls *arg = new TimerEpolls(timerCallback, (void *)get_curr_routine(), timeout, evNum, epollfd_, revents);

    std::cout << __FUNCTION__ << " : " << __LINE__ << "addEpoll  new--------------------------" << std::endl;

    for (unsigned long long i = 0; i < evNum; i++)
    {
        //初始化timerEvent的信息
        arg->timer_event_[i].timer_epolls_ = arg;
        arg->timer_event_[i].epollCallback_ = epollCallback;
        arg->timer_event_[i].selffd_ = evs[i].data.fd;
        arg->timer_event_[i].env_.events = evs[i].events;
        arg->timer_event_[i].env_.data.ptr = arg->timer_event_ + i;
        if (arg->timer_event_[i].selffd_ < 0)
            continue;
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, arg->timer_event_[i].selffd_, &arg->timer_event_[i].env_);
        assert(ret == 0);
    }
    //将 Timer_Epolls 定时事件加入定时器
    get_curr_thread_env()->time_heap_->AddTimer(arg);
    //退出当前协程
    get_curr_routine()->Yield();

    // 再回到当前协程
    //删除加入的socket
    for (unsigned long long i = 0; i < evNum; i++)
    {
        if (evs->data.fd < 0)
            continue;

        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, arg->timer_event_[i].selffd_,
                            &arg->timer_event_[i].env_);
        assert(ret == 0);
    }
    return arg->RaiseNum_;
}