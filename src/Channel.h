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
namespace Tattoo
{
	
using namespace std;

class EventLoop;
class Channel 
{
	typedef std::function<void()> EventCallback;		   // 事件回调函数
	typedef std::function<void(time_t)> ReadEventCallback; // 读操作回调函数，需要传入时间

  public:
	Channel(EventLoop *loop, int fd, Routine_t rou);

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

	int getFd() const { return fd_; }
	int getEvents() const { return events_; }
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

  private:
	/* 通过调用loop_->updateChannel()来注册或改变本fd在epoll中监听的事件,一律改为可读可写*/
	void update();

	static const int kNoneEvent;  //无事件
	static const int kReadEvent;  //可读事件
	static const int kWriteEvent; //可写事件

	EventLoop *loop_;
	const int fd_;
	int events_;  // 该fd正在监听的事件
	int revents_; // poll调用后，该fd需要处理的事件，依据它，poller调用它相应的回调函数
	bool addToLoop_;
	Routine_t *rou_; //与　Routine_t　对应

	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;
};
} // namespace Tattoo
#endif
