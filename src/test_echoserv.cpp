/*************************************************************************
	> File Name: liu_EchoServer.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月11日 星期二 16时51分55秒
 ************************************************************************/

#include "routine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <iostream>

using namespace std;
struct task_t
{
    stCoRoutine_t *co;
    int fd;
};
static stack<task_t *> g_readwrite;
static int g_listenfd = -1;

static void SetNonBlock(const int &fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}
static int CreateSocket(const char *ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd > 0)
    {
        int optval = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        struct sockaddr_in serv_addr;
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &serv_addr.sin_addr);
        serv_addr.sin_port = htons(port);
        bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        return fd;
    }
}
static void *fun_acc(void *arg)
{
    // co_enable_hook_sys();

    printf("accept  协程\n");

    fflush(stdout);
    while (1)
    {
        if (g_readwrite.empty())
        {
            printf("stack is empty \n");
            struct pollfd pf = {0};
            pf.fd = -1;
            // poll(&pf, 1, 1000);
            continue;
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t len = sizeof(addr);
        int fd = accept(g_listenfd, (struct sockaddr *)&addr, &len);

        printf("新的连接，fd = %d \n", fd);

        if (fd < 0)
        {
            struct pollfd pf = {0};
            pf.fd = g_listenfd;
            pf.events = (POLLIN | POLLERR | POLLHUP);
            co_poll(co_get_epoll_ct(), &pf, 1, 100000000);
            continue;
        }
        if (g_readwrite.empty())
        {
            close(fd);
            continue;
        }
        SetNonBlock(fd);
        task_t *co = g_readwrite.top();
        co->fd = fd;
        g_readwrite.pop();
        co_resume(co->co);
    }
    return 0;
}

static void *fun(void *arg)
{
    /*
	libco封装了系统调用，在系统调用，比如 send/recv/condition_wait 等函数前面加了一层 hook 
	有了这层 hook 就可以在系统调用的时候不让线程阻塞    而去进行线程切换*/

    printf("fun 函数运行---------\n");

    // co_enable_hook_sys();

    task_t *co = (task_t *)arg;
    char buf[1024 * 16];
    while (1)
    {
        if (-1 == co->fd) /*如果没有任务到来*/
        {
            g_readwrite.push(co);

            printf("该协程让出ＣＰＵ　\n");

            co_yield_ct(); /*让出ＣＰＵ，交给调用他的协程 for 循环　*/

            printf("产生连接******************************\n");

            continue;
        }

        int fd = co->fd;
        co->fd = -1;

        while (1)
        {
            struct pollfd pf = {0};
            pf.fd = fd;
            pf.events = (POLLIN | POLLERR | POLLHUP);
            /*
			int	co_poll( stCoEpoll_t *ctx,struct pollfd fds[], nfds_t nfds, int timeout_ms )
			{
				return co_poll_inner(ctx, fds, nfds, timeout_ms, NULL);
			}

参数1：eventloop控制块指针。因为poll要跟eventloop搭配。（数据结构在eventloop代码的上面）

参数2：监听的句柄数组，struct pollfd是Linux  poll用到的数据结构。

参数3：用于标记数组fds中的结构体元素的总数量。

参数4：超时时间ms，若此参数不为空，则此时间不仅仅被加入到epoll中监听，还会被加入到事件轮中等待超时同样会转为active。

参数5：函数指针，超时（active）事件，定义如下：

typedef int (*poll_pfn_t)(struct pollfd fds[], nfds_t nfds, int timeout);
			*/
            co_poll(co_get_epoll_ct(), &pf, 1, 1000);

            int ret = read(fd, buf, sizeof(buf));

            printf("读取到：%s\n", buf);

            if (ret > 0)
            {
                ret = write(fd, buf, ret);
            }
            if (ret > 0 || -1 == ret && EAGAIN == errno)
            {
                continue;
            }
            close(fd);
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    g_listenfd = CreateSocket(ip, port);
    listen(g_listenfd, 1024);
    printf("listen %d %s:%d\n", g_listenfd, ip, port);
    SetNonBlock(g_listenfd);

    for (int i = 0; i < 3; i++) /*相当于一个协程池*/
    {
        task_t *task = (task_t *)calloc(1, sizeof(task_t));
        task->fd = -1;
        /*
		CCB ,
		int co_create( stCoRoutine_t **ppco,const stCoRoutineAttr_t *attr,pfn_co_routine_t pfn,void *arg )
		*/
        co_create(&(task->co), NULL, fun, task); /*创建协程*/
        std::cout << "创建协程" << std::endl;

        co_resume(task->co); /*启用该协程*/
    }

    stCoRoutine_t *accept_co = NULL;
    co_create(&accept_co, NULL, fun_acc, 0);
    co_resume(accept_co);

    co_eventloop(co_get_epoll_ct(), 0, 0);

    exit(0);
}