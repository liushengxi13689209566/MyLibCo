/*************************************************************************
	> File Name: Channel.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月21日 星期一 17时21分38秒
 ************************************************************************/

#ifndef _CHANNEL_H
#define _CHANNEL_H
// #include "noncopyable.h"
#include <functional>
#include <ctime>
#include"EventLoop.h"

// #include"routine.h"

namespace Tattoo
{

using namespace std;
class EventLoop;

class Channel
{
  public:
	typedef std::function<void()> EventCallback; // 事件回调函数
	// typedef std::function<void(time_t)> ReadEventCallback; // 读操作回调函数，需要传入时间

	Channel(EventLoop *loop, int fd);

	~Channel();

	// 处理回调事件，一般由epoll通过eventLoop来调用
	void handleEvent();

	void setReadCallback(const EventCallback &cb)
	{
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback &cb)
	{
		writeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback &cb)
	{
		errorCallback_ = cb;
	}

	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; } // used by epoll
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	void enableReading()
	{
		events_ |= kReadEvent;
		update();
	}
	void enableWriting()
	{
		events_ |= kWriteEvent;
		update();
	}
	void disableAll()
	{
		events_ = kNoneEvent;
		update();
	}
	//	for Poller
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }

	EventLoop *ownerLoop() { return loop_; }

  private:
	/* 通过调用loop_->updateChannel()来注册或改变本fd在 epoll 中监听的事件,一律改为可读可写*/
	void update();

	static const int kNoneEvent;  //无事件
	static const int kReadEvent;  //可读事件
	static const int kWriteEvent; //可写事件

	EventLoop *loop_;
	const int fd_;
	int events_;  // 该fd正在监听的事件
	int revents_; // poll调用后，该fd需要处理的事件，依据它，poller调用它相应的回调函数
	int index_;   //  used by Poller

	// bool addToLoop_;
	// Routine_t *rou_; //与　Routine_t　对应

	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback errorCallback_;
};
} // namespace Tattoo
#endif
