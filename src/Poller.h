/*************************************************************************
	> File Name: Poller.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 16时34分13秒
 ************************************************************************/

#ifndef _POLLER_H
#define _POLLER_H
#include "Time_heap.h"

namespace Tattoo
{
class Poller
{
public:
	Poller(TimerFun timefun, void *arg, int delay);
	~Poller();

private:
	MiniHeap *miniheap_;
	TimerFun timerCallback_;
	void *arg_;
	int delay_;
};

} // namespace Tattoo

#endif
