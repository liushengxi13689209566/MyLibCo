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
// #include "routine.cpp"
#include "Time_heap.h"
#include "EventLoop.h"
#include "Poller.h"
#include "Poller.cpp"
#include "Log.h"
#include "Epoll.h"
#include "Epoll.cpp"
#include "callback.h"
using namespace Tattoo;

struct task
{
    Routine_t *routine;
    int fd;
};
static std::stack<task *> g_readwrite;
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
    task *tsk = (task *)arg;
    char buf[1024 * 10];
    for (;;)
    {
        //当本协程没有监听Socket是，把其放入协程等待队列
        if (tsk->fd == -1)
        {
            g_readwrite.push(tsk);
            get_curr_routine()->Yield();
        }

        //设置为-1表示已读，方便协程下次循环退出
        int fd = tsk->fd;
        tsk->fd = -1;

        struct epoll_event revents[1000];
        //不断监听
        //不断循环等待，等待读取信息
        for (;;)
        {
            struct epoll_event env;
            env.data.fd = fd;
            env.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
            int epollret = get_curr_thread_env()->epoll_->addEpoll(&env, 1, revents, 10);
            if (epollret == 0)
            {
                continue;
            }

            int ret = read(fd, buf, sizeof(buf));
            if (ret > 0)
            {
                std::cout << __FUNCTION__ << __LINE__ << "ret : " << ret << std::endl;
                std::cout << __FUNCTION__ << __LINE__ << "message :: " << buf << std::endl;
                //ret = write( fd,buf,ret );
            }
            else
            {
                close(fd);
                break;
            }
        }
    }
}

static void *accept_routine(void *)
{
    for (;;)
    {
        std::cout << "accepter ::" << std::endl;
        if (g_readwrite.empty())
        {
            std::cout << "There is no routine to read socket! " << std::endl;
            struct epoll_event ev;
            ev.data.fd = -1;
            ev.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
            struct epoll_event revents[10];
            int epollret = get_curr_thread_env()->epoll_->addEpoll(&ev, 1, revents, 10);
            continue;
        }

        struct sockaddr_in addr; //maybe sockaddr_un;
        memset(&addr, 0, sizeof(addr));
        socklen_t len = sizeof(addr);
        int fd = accept(g_listen_fd, (struct sockaddr *)&addr, &len);

        if (fd < 0)
        {
            struct epoll_event ev;
            ev.data.fd = g_listen_fd;
            ev.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
            struct epoll_event revents[10];
            int epollret = get_curr_thread_env()->epoll_->addEpoll(&ev, 1, revents, 10);
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
        task *tsk = g_readwrite.top();
        tsk->fd = fd;
        tsk->routine->Resume();
        g_readwrite.pop();
    }
}

//典型的Reactor模式

int main()
{
    int backlog;
    int portnumber = 12349;
    char *local_addr = "127.0.0.1";

    g_listen_fd = CreateTcpSocket(portnumber, local_addr, true);
    listen(g_listen_fd, 1024);

    SetNonBlock(g_listen_fd);

    //在每个进程中创建协程
    std::vector<Routine_t *> Routine_tArr;
    int number = 10;

    for (int i = 0; i < number; i++)
    {
        task *tsk = (task *)calloc(1, sizeof(task));
        tsk->fd = -1;
        Routine_tArr.push_back(new Routine_t(get_curr_thread_env(), NULL, readwrite_routine, tsk));
        tsk->routine = Routine_tArr[i];
    }

    for (int i = 0; i < number; i++)
    {
        Routine_tArr[i]->Resume();
    }

    Routine_t *accepter = (new Routine_t(get_curr_thread_env(), NULL, accept_routine, NULL));
    accepter->Resume();

    EventLoop eventloop(get_curr_thread_env()->time_heap_, NULL, NULL);
    eventloop.loop();
    return 0;
}
