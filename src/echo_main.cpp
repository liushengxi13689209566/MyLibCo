/*************************************************************************
	> File Name: echo_main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月20日 星期三 14时40分19秒
 ************************************************************************/

#include <iostream>
#include "TcpServer.h"
#include "TcpServer.cpp"

#include <iostream>
#include <unistd.h>
#include <stdio.h>

using namespace Tattoo;

void onConnection(void *)
{
	std::cout << "a new connection " << std::endl;
}

void onMessage(const int fd)
{
	char buf[1204 * 16] = {0};
	int ret = read(fd, buf, sizeof(buf));
	if (ret > 0)
	{
		ret = write(fd, buf, ret);
		std::cout << "message :: " << buf << std::endl;
	}
	else
	{
		close(fd);
	}
}
int main(int argc, char *argv[])
{
	printf("main(): pid = %d\n", getpid());
	//ip   = 127.0.0.1
	//port = 9981
	if (argc <= 2)
	{
		cout << "usage : " << basename(argv[0]) << "   ip_address    port_number   " << endl;
		return 1;
	}
	const char *ip = argv[1];
	const int port = atoi(argv[2]);

	//2  进程　
	//10 协程
	TcpServer server(ip, port, 2, 10);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.start();
}