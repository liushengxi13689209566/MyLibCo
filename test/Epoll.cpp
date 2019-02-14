
#include "Epoll.h"
// #include "Log.h"
#include "routine.h"
#include <sys/epoll.h>
#include <assert.h>
using namespace Tattoo;
Epoll::Epoll(EventLoop *loop)
    : epollfd_(epoll_create(kInitEventListSize)),
      owerLoop_(loop)
{
}
Epoll::~Epoll()
{
}

Timestamp Epoll::poll(int timeoutMs, ChannelList *activeChannels)
{
    //调用epoll_wait
    //events是一个EPollPoller中struct epoll_event的vector私有变量
    int numEvents = ::epoll_wait(epollfd_,
                                 &*eventfds_.begin(), //可以使用eventfds_.data()返回指向第一个成员的指针
                                 static_cast<int>(eventfds_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    time_t time(NULL);
    if (numEvents > 0)
    {
        //更新Channel列表
        fillActiveChannels(numEvents, activeChannels);

        if (implicit_cast<size_t>(numEvents) == eventfds_.size())
        {
            eventfds_.resize(eventfds_.size() * 2); //如果返回的事件数目等于当前事件数组大小，就分配2倍空间
        }
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
        //     LOG_SYSERR << "EPollPoller::poll()";
        // }
    }
}
//更新事件列表
void Epoll::fillActiveChannels(int numEvents,
                               ChannelList *activeChannels) const
{
    assert(implicit_cast<size_t>(numEvents) <= eventfds_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        //epoll_wait会原封不动返回 ptr 指向的结构体
        Channel *channel = static_cast<Channel *>(eventfds_[i].data.ptr);

        int fd = channel->fd();                             //拿到其中的文件描述符
        ChannelMap::const_iterator it = channels_.find(fd); //ChannelMap以文件描述符为key
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->set_revents(eventfds_[i].events); //将事件类型赋值给Channel类中reventfds_元素
        activeChannels->push_back(channel);        //添加进就绪事件合集
    }
}