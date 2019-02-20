/*************************************************************************
	> File Name: test_echoserv.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月19日 星期二 17时26分20秒
 ************************************************************************/

#include <iostream>
// #include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

// #include "Acceptor.cpp"
#include "EventLoop.cpp"
#include "InetAddress.cpp"
#include "SocketsOps.cpp"
#include "Socket.cpp"
#include "Channel.cpp"
#include "Timestamp.cpp"
#include "MiniHeap.cpp"
#include "Epoll.cpp"
#include "routine.h"
#include "routine.cpp"
#include <stack>
#include "debug.h"

using namespace std;
struct task_t
{
	Routine_t *routine;
	int fd;
};

static std::stack<task_t *> g_readwrite;
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
	INFO("进入 readwrite_routine()");

	task_t *tsk = (task_t *)arg;
	char buf[1024 * 10];
	for (;;)
	{
		//当本协程没有监听Sockets，把其放入协程等待队列
		if (tsk->fd == -1)
		{
			g_readwrite.push(tsk);
			INFO("调用 Yield()");
			get_curr_routine()->Yield();
			continue;
		}
		//设置为-1表示已读，方便协程下次循环退出
		INFO("come back to readWrite routine() ");

		int fd = tsk->fd;
		tsk->fd = -1;
		//不断监听
		//不断循环等待，等待读取信息
		for (;;)
		{
			// struct epoll_event env;
			// env.data.fd = fd;
			// env.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
			// int epollret = get_curr_thread_env()->epoll_->addEpoll(&env, 1, revents, 10);

			Channel chan(get_curr_thread_env()->envEventLoop_, fd);
			chan.addEpoll();
			//注册事件，并退出 yield()

			// data.ptr 对应　channel ,而　channel 中要对应 Routine_t ,这样当有数据什么的到来时，就直接唤醒对应的协程即可
			INFO("注册事件，并退出 readWrite routine() ,返回　main 函数");

			int ret = read(fd, buf, sizeof(buf));
			if (ret > 0)
			{
				ret = write(fd, buf, ret);
				std::cout << "message :: " << buf << std::endl;
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
	for (;;)
	{
		std::cout << "accepter ::" << std::endl;

		struct sockaddr_in addr; //maybe sockaddr_un;
		memset(&addr, 0, sizeof(addr));
		socklen_t len = sizeof(addr);
		int fd = accept(g_listen_fd, (struct sockaddr *)&addr, &len);
		printf("accept fd == %d\n", fd); //重复添加了 g_listen_fd  导致服务器退出
		if (fd < 0)
		{
			Channel chan(get_curr_thread_env()->envEventLoop_, g_listen_fd);
			// struct epoll_event ev;
			// ev.data.fd = g_listen_fd;
			// ev.events = (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
			// struct epoll_event revents[10];
			chan.addEpoll(); //注册事件，并退出 yield()
			continue;
		}
		// fd > 0
		if (g_readwrite.empty())
		{
			close(fd);
			continue;
		}

		SetNonBlock(fd);
		std::cout << __FUNCTION__ << " a New Connection !" << std::endl;
		//accept完后启动其他协程

		//取出最顶端的空闲协程执行对应的 fd 读写
		task_t *tsk = g_readwrite.top();
		tsk->fd = fd;
		g_readwrite.pop();

		INFO("调用Resume()函数");

		tsk->routine->Resume();
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
	int number = 10;

	EventLoop eventloop;
	std::vector<Routine_t *> RoutineArr;

	for (int i = 0; i < number; i++)
	{
		task_t *tsk = (task_t *)calloc(1, sizeof(task_t));
		tsk->fd = -1;

		RoutineArr.push_back(new Routine_t(get_curr_thread_env(), NULL, readwrite_routine, tsk));
		tsk->routine = RoutineArr[i];

		RoutineArr[i]->Resume();
	}

	Routine_t *accepter = (new Routine_t(get_curr_thread_env(), NULL, accept_routine, NULL));
	accepter->Resume();

	eventloop.loop();

	return 0;
}
