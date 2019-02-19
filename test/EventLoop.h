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

// #include "routine.cpp"

// 这是Reactor模式的核心，每个Reactor线程内部调用一个EventLoop，
// 内部不停的进行poll或者epoll_wait调用，然后根据fd的返回事件，
// 调用fd对应Channel的相应回调函数./test

namespace Tattoo
{

class Channel;
class Epoll;
class TimeHeap;

class EventLoop
{
  public:
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();

	void loop();

	Timestamp pollReturnTime() const { return pollReturnTime_; }

	/// Runs callback immediately in the loop thread.
	/// It wakes up the loop, and run the cb.
	/// If in the same loop thread, cb is run within the function.
	/// Safe to call from other threads.
	void runInLoop(const Functor &cb);
	/// Queues callback in the loop thread.
	/// Runs after finish pooling.
	/// Safe to call from other threads.
	void queueInLoop(const Functor &cb);

	// timers

	///
	/// Runs callback at 'time'.
	/// Safe to call from other threads.
	///
	void runAt(const Timestamp &time, const TimerCallback &cb);
	///
	/// Runs callback after @c delay seconds.
	/// Safe to call from other threads.
	///
	void runAfter(double delay, const TimerCallback &cb);
	///
	/// Runs callback every @c interval seconds.
	/// Safe to call from other threads.
	///
	void runEvery(double interval, const TimerCallback &cb);

	// void cancel(TimerId timerId);

	// internal use only
	void updateChannel(Channel *channel);
	// void removeChannel(Channel* channel);

  private:
	typedef std::vector<Channel *> ChannelList;

	bool looping_; /* atomic */

	Timestamp pollReturnTime_;
	Epoll *epoll_;
	TimeHeap *timerHeap_;

	ChannelList activeChannels_;
	Functor pendingFunctors_;
	RoutineEnv_t *rouEnv_;
};
} // namespace Tattoo

#endif
