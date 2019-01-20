/*************************************************************************
	> File Name: eventloop.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 10时53分11秒
 ************************************************************************/

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H
#include <cstdint>
#include <sys/epoll.h>

class BaseEvent
{
  public:
	BaseEvent();
	virtual ~BaseEvent(){};
	void SetEvent(uint32_t ev) { events_ = ev; }
	uint32_t GetEvents() { return events_; }

  protected:
	uint32_t events_;
};
#endif
