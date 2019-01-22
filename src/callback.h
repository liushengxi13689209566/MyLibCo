/*************************************************************************
	> File Name: liu_callback.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月16日 星期三 17时01分56秒
 ************************************************************************/

#ifndef _CALLBACK_H
#define _CALLBACK_H
namespace Tattoo
{
//协程函数
typedef void *(*RoutineFun)(void *);

//定时器回调函数
typedef void *(*TimerFun)(void *);

} // namespace Tattoo
#endif
