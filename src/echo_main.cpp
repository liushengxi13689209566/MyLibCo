/*************************************************************************
	> File Name: echo_main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月20日 星期三 14时40分19秒
 ************************************************************************/

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "TcpServer.h"
#include "TcpServer.cpp"
using namespace Tattoo;

void onConnection(void *)
{
    std::cout << " a new connection " << std::endl;
}

void onMessage(const int fd)
{
    std::cout << " get a message " << std::endl;
}
int main(int argc, char *argv[])
{
    int port = 9981;
    char *ip = "127.0.0.1";

    if (argc > 1)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }
    printf("main(): pid == %d, %s:%d\n", getpid(), ip, port);

    //创建一个　server，里面包含2个进程，每个进程中又拥有10个协程
    TcpServer server(ip, port, 2, 10);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    return 0;
}