/*************************************************************************
	> File Name: EventLoop.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月22日 星期二 09时45分18秒
 ************************************************************************/

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H
#include "noncopyable.h"

// 这是Reactor模式的核心，每个Reactor线程内部调用一个EventLoop，
// 内部不停的进行poll或者epoll_wait调用，然后根据fd的返回事件，
// 调用fd对应Channel的相应回调函数
namespace Tattoo
{
class EventLoop
{
public:
	EventLoop();
	~EventLoop();

private:
};
} // namespace Tattoo

#endif
