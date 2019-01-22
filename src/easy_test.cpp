//test for Conditional_variable

#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <iostream>
#include "routine.h"
#include "routine.cpp"
#include "Time_heap.h"
#include "Time_heap.cpp"
#include "Poller.h"
#include "Poller.cpp"
// #include "Log.h"
#include "Epoll.h"
#include "Epoll.cpp"
#include "Callback.h"
using namespace libfly;

//设置非阻塞Socket
static int SetNonBlock(int iSock)
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

//设置地址
static void SetAddr(const char *pszIP, const unsigned short shPort, struct sockaddr_in &addr)
{
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(shPort);
    int nIP = 0;
    if (!pszIP || '\0' == *pszIP || 0 == strcmp(pszIP, "0") || 0 == strcmp(pszIP, "0.0.0.0") || 0 == strcmp(pszIP, "*"))
    {
        nIP = htonl(INADDR_ANY);
    }
    else
    {
        nIP = inet_addr(pszIP);
    }
    addr.sin_addr.s_addr = nIP;
}

//创建socket
static int CreateTcpSocket(const unsigned short shPort = 0, const char *pszIP = "*", bool bReuse = false)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd >= 0)
    {
        if (shPort != 0)
        {
            if (bReuse)
            {
                int nReuseAddr = 1;
                setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, sizeof(nReuseAddr));
            }
            struct sockaddr_in addr;
            SetAddr(pszIP, shPort, addr);
            int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
            if (ret != 0)
            {
                close(fd);
                return -1;
            }
        }
    }
    return fd;
}

void *timerCallback(Routine *routine)
{
    //std::cout<<"I'm timerCallback Function. I will resume this routine!\n"<<std::endl;
    routine->resume();
}

void *Poll(void *arg)
{
    int backlog;
    int portnumber = 12349;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    char *local_addr = "127.0.0.1";
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_aton(local_addr, &(serveraddr.sin_addr)); //htons(portnumber);
    serveraddr.sin_port = htons(portnumber);

    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;

    bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, backlog);

    struct epoll_event revents[1000];
    while (1)
    {
        ev.data.fd = listenfd;
        ev.events = EPOLLIN | EPOLLET;
        int ret = get_curr_thread_env()->epoll_->addEpoll(&ev, 1, revents, 100);

        struct sockaddr_in addr; //maybe sockaddr_un;
        memset(&addr, 0, sizeof(addr));
        socklen_t len = sizeof(addr);

        for (int i = 0; i < ret; i++)
        {
            std::cout << revents[i].data.fd << std::endl;
        }
        std::cout << "I'm back!" << std::endl;

        int fd = accept(listenfd, (struct sockaddr *)&addr, &len);
        std::cout << "Accept::fd " << fd << std::endl;
        char buf[1000];
        int rett = read(fd, buf, sizeof(buf));
        std::cout << "Size :" << rett << " message :" << buf << std::endl;
    }
}

int main()
{

    std::vector<Routine *> RoutineArr;
    RoutineArr.push_back(new Routine(get_curr_thread_env(), NULL, Poll, NULL));
    for (int i = 0; i < 1; i++)
    {
        RoutineArr[i]->resume();
    }
    std::cout << "I'm Main routine" << std::endl;
    EventLoop eventloop(get_curr_thread_env()->time_heap_, NULL, NULL);
    eventloop.loop();
    return 0;
}
