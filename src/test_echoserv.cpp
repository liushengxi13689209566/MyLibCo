//test for Server

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
#include <stack>
#include <iostream>
#include "routine.h"
#include "Time_heap.h"
#include "EventLoop.h"
#include "Poller.h"
#include "Poller.cpp"
#include "Log.h"
#include "Epoll.h"
#include "Epoll.cpp"
#include "callback.h"
#include "Channel.h"
using namespace Tattoo;

static std::stack<Channel *> g_readwrite;
static int g_listen_fd = -1;

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

static void *readwrite_routine(void *arg)
{
    Channel *chan = (Channel *)arg;
    char buf[1024 * 10];
    for (;;)
    {
        //channel与Routine_t还没有绑定，把其放入协程等待队列
        if (chan->fd_ == -1)
        {
            g_readwrite.push(chan);
            get_curr_routine()->Yield(); //co_yield_ct() ;
            continue;
        }

        //设置为-1表示已读，方便协程下次循环退出
        int fd = chan->fd();
        chan->fd_ = -1;

        for (;;)
        {

            Channel Channel(get_curr_eventloop(), fd); //将新建立的连接的 fd 加入到 Epoll 监听中,并设置可读可写

            /* 将新建立的连接的 fd 加入到 Epoll 监听中，并将控制流程返回到 main 协程；
            当有读或者写事件发生时，Epoll 会唤醒对应的 coroutine ，继续执行 read 函数以及 write 函数。*/

            int ret = read(fd, buf, sizeof(buf));
            if (ret > 0)
            {
                std::cout << __FUNCTION__ << __LINE__ << "ret : " << ret << std::endl;
                std::cout << __FUNCTION__ << __LINE__ << "message :: " << buf << std::endl;
                ret = write(fd, buf, ret);
            }
            if (ret > 0 || (-1 == ret && EAGAIN == errno))
            {
                continue;
            }
            close(fd);
            break;
        }
    }
}

static void *accept_routine(void *)
{
    std::cout << "accepter ::" << std::endl;
    for (;;)
    {
        struct sockaddr_in addr; //maybe sockaddr_un;
        memset(&addr, 0, sizeof(addr));
        socklen_t len = sizeof(addr);
        int fd = accept(g_listen_fd, (struct sockaddr *)&addr, &len);
        if (fd < 0)
        {
            //如果接收连接失败，那么调用 co_poll 将服务端的 listen_fd 加入到 Epoll 中来触发下一次连接事件
            struct epoll_event ev;
            ev.data.fd = g_listen_fd;
            ev.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
            struct epoll_event revents[10];
            // std::cout << __FUNCTION__ << " : " << __LINE__ << " a New Connection !" << std::endl;
            std::cout << __FUNCTION__ << " : " << __LINE__ << " add epoll success " << std::endl;
            continue;
        }
        if (g_readwrite.empty())
        {
            close(fd);
            continue;
        }

        SetNonBlock(fd);
        std::cout << __FUNCTION__ << " a New Connection !" << std::endl;
        //accept完后启动其他协程

        //取出最顶端的空闲协程执行对应的fd读写
        Channel *chan = g_readwrite.top();
        chan->fd = fd;
        chan->routine->Resume();
        g_readwrite.pop();
    }
}

//典型的Reactor模式

int main(int argc, char *argv[])
{
    EventLoop eventloop;

    int backlog;
    char *local_addr = argv[1];
    int portnumber = atoi(argv[2]);

    g_listen_fd = CreateTcpSocket(portnumber, local_addr, true);
    listen(g_listen_fd, 1024);

    SetNonBlock(g_listen_fd);

    std::vector<Routine_t *> Routine_tArr;
    int number = 10;
    for (int i = 0; i < number; i++)
    {
        Channel *chan = new Channel(readwrite_routine);
        Routine_tArr.push_back(new Routine_t(get_curr_thread_env(), NULL, readwrite_routine, chan));
        chan->routine = Routine_tArr[i];
    }
    for (int i = 0; i < number; i++)
    {
        Routine_tArr[i]->Resume();
    }

    Routine_t *accepter = (new Routine_t(get_curr_thread_env(), NULL, accept_routine, NULL));
    accepter->Resume();
    eventloop.loop();
    
    return 0;
}
