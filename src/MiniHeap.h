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
    Timer(Timestamp when);
    Timestamp expiration() const { return expire_; }
    void run() const;

    Timestamp expire_; //任务的超时事件
    Routine_t *timer_rou_;
};

class TimeHeap
{
  public:
    TimeHeap(EventLoop *loop);
    ~TimeHeap();

    Timer *addTimer(Timestamp when);
    void delTimer(Timer *timer);

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
