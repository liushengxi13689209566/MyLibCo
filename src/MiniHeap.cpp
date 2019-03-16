/*************************************************************************
	> File Name: Time_heap.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 16时49分00秒
 ************************************************************************/

#include "MiniHeap.h"

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
//创建　timerfd
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
/* 计算超时时间与当前时间的时间差,并将参数转换为 api 接受的类型  */
struct timespec howMuchTimeFromNow(Timestamp when)
{
    /* 微秒数 = 超时时刻微秒数 - 当前时刻微秒数 */
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts; // 转换成 struct timespec 结构返回
    //　tv_sec 秒
    //　tv_nsec 纳秒
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
/* 读timerfd，避免定时器事件一直触发 */
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
/* 重置 timerfd 的超时时间 */
void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    //到这个时间后，会产生一个定时事件
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

Timer::Timer(Timestamp when)
    : timer_rou_(get_curr_routine()), //一个定时器对应一个协程
      expire_(when)
{
}
void Timer::run() const
{
    cout << "由定时器唤醒对应协程" << endl;
    timer_rou_->Resume();
}

TimeHeap::TimeHeap(EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_()
{
    // 设置自己独特的回调函数，并不是和普通的Channel 一样，直接唤醒了对应的协程
    timerfdChannel_.setHandleCallback(
        std::bind(&TimeHeap::handleRead, this));
    timerfdChannel_.enableReading();
}
TimeHeap::~TimeHeap()
{
    timerfdChannel_.disableAll();
    ::close(timerfd_);
    for (auto it = timers_.begin();
         it != timers_.end(); ++it)
    {
        delete it->second;
    }
}
/* 添加一个定时器 ,返回定时器指针，会在 channel->addEpoll 函数中使用到，因为要删除对应的定时器*/
Timer *TimeHeap::addTimer(Timestamp when)
{
    Timer *timer = new Timer(when);
    ////如果当前插入的定时器 比队列中的定时器都早 则返回真
    bool earliestChanged = insert(timer);
    //最早的超时时间改变了，就需要重置timerfd_的超时时间
    if (earliestChanged)
    {
        //timerfd_ 重新设置超时时间,使得 timerfd  的定时事件始终是最小的
        resetTimerfd(timerfd_, timer->expiration());
    }
    return timer;
}
/* 删除一个定时器 */
void TimeHeap::delTimer(Timer *timer)
{
    auto it = timers_.find(timer->expire_);
    if (it != timers_.end())
    {
        timers_.erase(it);
    }
    return;
}
//timerfd 可读 的回调
void TimeHeap::handleRead()
{
    Timestamp now(Timestamp::now());
    //先读取
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        it->second->run(); //run->Resume()
    }
    reset(expired, now); //这里主要是改变 timerfd 的定时最小值
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
        delete it->second;
    }
    if (!timers_.empty()) //timers_ 不为空
    {
        /*获取当前定时器集合中的最早定时器的时间戳，作为下次超时时间*/
        nextExpire = timers_.begin()->second->expiration();
    }
    //如果取得的时间 >0就改变 timerfd 的定时
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