
#include "Channel.h"
#include <iostream>
#include <poll.h>
#include <stdio.h>
/*　
    POLLIN 　　　　　　　　有数据可读。

　　POLLRDNORM 　　　　  有普通数据可读。

　　POLLRDBAND　　　　　 有优先数据可读。

　　POLLPRI　　　　　　　　 有紧迫数据可读。

　　POLLOUT　　　　　　      写数据不会导致阻塞。

　　POLLWRNORM　　　　　  写普通数据不会导致阻塞。

　　POLLWRBAND　　　　　   写优先数据不会导致阻塞。

　　POLLMSGSIGPOLL 　　　　消息可用。
*/
using namespace Tattoo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      index_(-1),
      channelRoutine_(get_curr_routine())
{
}
Channel::~Channel()
{
}
void Channel::update()
{
    // printf("index_ == %d\n", index_);
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    if (handleCallback_)
        handleCallback_();
}
void Channel::handleFun()
{
    if (channelRoutine_)
        channelRoutine_->Resume();
}
void Channel::addEpoll()
{
    setHandleCallback(std::bind(&Channel::handleFun,this));
    events_ |= kReadEvent;
    events_ |= kWriteEvent;
    //Channel::update()->EventLoop::updateChannel(Channel*)->Poller::updateChannel(Channel*)
    update();
    Timer *tmp = loop_->runAfter(10);
    //退出当前协程
    get_curr_routine()->Yield();
    // fixme 删除加入的　epoll 信息和定时器
    loop_->removeChannel(this);
    loop_->cancel(tmp);
}
