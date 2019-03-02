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
    std::cout << " a new connection " << std::endl;
}

void onMessage(const int fd)
{
    std::cout << " get a message " << std::endl;
}
int main(int argc, char *argv[])
{
    //ip   = 127.0.0.1
    //port = 9981
    int port = 9981;
    char *ip = "127.0.0.1";

    if (argc > 1)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    //2  进程　
    //10 协程
    printf("main(): pid == %d, %s:%d\n", getpid(), ip, port);

    TcpServer server(ip, port, 2, 10);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
}