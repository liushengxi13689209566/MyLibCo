
#include "Channel.h"
#include <iostream>
#include <poll.h>

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
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    // if (revents_ & POLLNVAL)
    // {
    //     std::cout << "Channel::handle_event() POLLNVAL" << std::endl;
    // }

    // if (revents_ & (POLLERR | POLLNVAL))
    // {
    //     if (errorCallback_)
    //         errorCallback_();
    // }
    // if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    // {
    //     if (readCallback_)
    //         readCallback_();
    // }
    // if (revents_ & POLLOUT)
    // {
    //     if (writeCallback_)
    //         writeCallback_();
    // }
    
    // 当有事件当来时直接唤醒对应的协程即可　
    if(channelRoutine_)
        channelRoutine_->Resume();
}
void Channel::addEpoll()
{
    //Channel::update()->EventLoop::updateChannel(Channel*)->Poller::updateChannel(Channel*)
    update();
    //退出当前协程
    get_curr_routine()->Yield();
    // fixme 删除加入的　epoll 信息
}
