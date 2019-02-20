
#include "Epoll.h"
#include <stdio.h>
// #include "routine.h"

#include <sys/epoll.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include "Channel.h"

using namespace Tattoo;

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

Epoll::Epoll(EventLoop *loop)
    : owerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
}
Epoll::~Epoll()
{
    ::close(epollfd_);
}

int Epoll::poll(int timeoutMs, ChannelList *activeChannels)
{
    //调用epoll_wait
    //events是一个Epoll中 struct epoll_event的vector私有变量
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(), //可以使用events_.data() 返回指向第一个成员的指针
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;

    printf("numEvents== %d \n", numEvents);

    // Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        //更新Channel列表
        fillActiveChannels(numEvents, activeChannels); //填充 activeChannels
    }
    else if (numEvents == 0)
    {
        std::cout << "nothing happened" << std::endl;
    }
    else
    {
        // error happens, log uncommon ones
        // if (savedErrno != EINTR)
        // {
        //     errno = savedErrno;
        //     LOG_SYSERR << "Epoll::poll()";
        // }
    }
    return numEvents;
}
//更新事件列表
void Epoll::fillActiveChannels(int numEvents,
                               ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        //epoll_wait 会原封不动返回 ptr 指向的结构体
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);

        channel->set_revents(events_[i].events); //将事件类型赋值给Channel类中 revents_ 元素
        activeChannels->push_back(channel);      //添加进就绪事件合集
    }
}
// 更新　
void Epoll::updateChannel(Channel *channel)
{
    const int index = channel->index();
    if (index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void Epoll::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    // LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    // assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}
void Epoll::update(int operation, Channel *channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel; //channel 对应　Routine_t ,直接唤醒对应协程
    int fd = channel->fd();
    ::epoll_ctl(epollfd_, operation, fd, &event);
}
const char *Epoll::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}