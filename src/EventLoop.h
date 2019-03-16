/*************************************************************************
	> File Name: EventLoop.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 09时45分18秒
 ************************************************************************/

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H

#include "Callbacks.h"
#include "Timestamp.h"
#include <vector>
#include <functional>
#include "routine.h"
namespace Tattoo
{

class Channel;
class Epoll;
class TimeHeap;
class Timer;
class RoutineEnv_t;

class EventLoop
{
  public:
    EventLoop();
    ~EventLoop();

    void loop();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // timers
    Timer *runAt(const Timestamp &time);
    Timer *runAfter(double delay);
    void cancel(Timer *timer);

    // internal use only
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

  private:
    typedef std::vector<Channel *> ChannelList;

    bool looping_; /* atomic */

    Timestamp pollReturnTime_;
    Epoll *epoll_;
    TimeHeap *timerHeap_;

    ChannelList activeChannels_;
    RoutineEnv_t *rouEnv_;
};
} // namespace Tattoo

#endif
