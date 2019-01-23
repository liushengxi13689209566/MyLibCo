
#include "Epoll.h"
#include <sys/epoll.h>

using namespace Tattoo;

Epoll::Epoll()
{
    epollfd_ = ::epoll_create(kInitEventListSize);
}
Epoll::~Epoll()
{
}