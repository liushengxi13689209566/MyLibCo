
#include "Channel.h"
#include <iostream>

using namespace Tattoo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      index_(-1),
      eventHandling_(false),
      addToLoop_(false)
{
}
Channel::~Channel()
{
    assert(!eventHandling_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}
void Channel::update()
{
    addToLoop_ = true;
    loop_->updateChannel(this);
}
void Channel::remove()
{
    assert(isNoneEvent());
    addToLoop_ = false;
    loop_->removeChannel(this);
}
void Channel::handleEvent(time_t receiveTime)
{
    eventHandling_ = true;
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (closeCallback_)
            closeCallback_();
    }

    if (revents_ & POLLNVAL)
    {
        std::cout << "fd = " << fd_ << " Channel::handle_event() POLLNVAL" << std::endl;
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_)
            readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
    eventHandling_ = false;
}
