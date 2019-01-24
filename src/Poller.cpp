
#include "Poller.h"
#include "routine.h"

using namespace Tattoo;

Poller::Poller(TimerFun timefun, void *arg, int delay)
    : miniheap_(get_curr_thread_env()->time_heap_),
      timerCallback_(timefun),
      arg_(arg),
      delay_(delay)
{
    if (delay_ < 0)
        delay_ = 1e9;
    miniheap_->AddTimer(timerCallback_, arg_, delay_);
    get_curr_routine()->Yield();
}

Poller::~Poller() {}