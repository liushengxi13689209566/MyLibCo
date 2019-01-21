/*************************************************************************
	> File Name: Channel.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月21日 星期一 17时21分38秒
 ************************************************************************/

#ifndef _CHANNEL_H
#define _CHANNEL_H
#include "noncopyable.h"
#include <functional>
#include <ctime>
namespace Tattoo
{
class EventLoop;

class Channel : noncopyable
{
	typedef std::function<void()> EventCallback;		   // 事件回调函数
	typedef std::function<void(time_t)> ReadEventCallback; // 读操作回调函数，需要传入时间

  public:
	Channel(EventLoop *loop, int fd);
	~Channel();

	// 处理回调事件，一般由epoll通过eventLoop来调用
	void handleEvent(time_t receiveTime);

	void setReadCallback(ReadEventCallback cb)
	{
		readCallback_ = std::move(cb);
	}
	void setWriteCallback(EventCallback cb)
	{
		writeCallback_ = std::move(cb);
	}
	void setCloseCallback(EventCallback cb)
	{
		closeCallback_ = std::move(cb);
	}
	void setErrorCallback(EventCallback cb)
	{
		errorCallback_ = std::move(cb);
	}

	int getFd() const { return fd_; }
	int getEvents() const { return events_; }
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	void enableReading()
	{
		events_ |= kReadEvent;
		update();
	}
	void disableReading()
	{
		events_ &= ~kReadEvent;
		update();
	}
	void enableWriting()
	{
		events_ |= kWriteEvent;
		update();
	}
	void disableWriting()
	{
		events_ &= ~kWriteEvent;
		update();
	}
	void disableAll()
	{
		events_ = kNoneEvent;
		update();
	}
	bool isWriting() const { return events_ & kWriteEvent; }
	bool isReading() const { return events_ & kReadEvent; }

	EventLoop *getEventLoop() { return loop_; }
	void remove();

  private:
	/* 通过调用loop_->updateChannel()来注册或改变本fd在epoll中监听的事件 */
	void update();

	static const int kNoneEvent;  //无事件
	static const int kReadEvent;  //可读事件
	static const int kWriteEvent; //可写事件

	EventLoop *loop_;
	const int fd_;
	int events_;		 // 该fd正在监听的事件
	int revents_;		 // poll调用后，该fd需要处理的事件，依据它，poller调用它相应的回调函数
	int index_;			 // used by Poller.
	bool logHup_;		 // 是否生成某些日志
	bool eventHandling_; // 是否正在处理事件
	bool addToLoop_;

	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;
};
} // namespace Tattoo
#endif
