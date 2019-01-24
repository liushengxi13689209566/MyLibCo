
#include "Epoll.h"
#include <sys/epoll.h>
#include <iostream>
#include <cassert>
#include "Channel.h"
#include <unistd.h>
#include <cerrno>

using namespace Tattoo;

namespace
{
const int kNew = 1;
const int kAdded = 1;
const int kDeleted = 2;
} // namespace

Epoll::Epoll(EventLoop *loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create(5)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0)
        std::cout << "Epoll::Epoll failed" << std::endl;
}
Epoll::~Epoll()
{
    ::close(epollfd_);
}
time_t Epoll::pool(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(
        epollfd_,
        &*events_.begin(),
        static_cast<int>(events_.size()),
        timeoutMs);
    int saveErrno = errno;
    time_t now = time(0);
    if (numEvents > 0)
    {
        std::cout << "events happened" << std::endl;
        fillActiveChannels(numEvents, activeChannels);
        if ((size_t)numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        std::cout << "nothing happened" << std::endl;
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            std::cout << "error in epoll_wait " << std::endl;
        }
    }
}
void Epoll::fillActiveChannels(
    int numEvents,
    ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr); // ptr 与 fd 有关的用户数据
        int fd = channel->getFd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);

        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}
void Epoll::updateChannel(Channel *channel)
{
    // assertInLoopThread();
    const int index = channel->getIndex();
    std::cout << "fd = " << channel->getFd()
              << " events = " << channel->getEvents() << " index = " << index << std::endl;
    if (index == kNew || index == kDeleted)
    {
        //a new one, add with EPOLL_CTL_ADD
        int fd = channel->getFd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else //  index = kDelete
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
    int fd = channel->getFd();
    std::cout << "fd = " << fd << std::endl;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->getIndex();
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
    bzero(&event, sizeof event);
    event.events = channel->getEvents();
    event.data.ptr = channel;
    int fd = channel->getFd();
    std::cout << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << std::endl;
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            std::cout << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd << std::endl;
        }
        else
        {
            std::cout << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd << std::endl;
        }
    }
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
