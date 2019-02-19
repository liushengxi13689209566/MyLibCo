/*************************************************************************
	> File Name: Callbacks.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月14日 星期四 18时14分54秒
 ************************************************************************/

#ifndef _CALLBACKS_H
#define _CALLBACKS_H
#include <functional>
#include "Timestamp.h"

namespace Tattoo
{
class Buffer;
class TcpConnection;

// All client visible callbacks go here.

typedef std::function<void()> TimerCallback;
// typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
// typedef std::function<void(const TcpConnectionPtr &,
						//    Buffer *buf,
						//    Timestamp)>
	// MessageCallback;

//协程函数
typedef void *(*RoutineFun)(void *);


} // namespace Tattoo
#endif
