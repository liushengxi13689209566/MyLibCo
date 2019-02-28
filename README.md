
简体中文 | [English](./README.zh-English.md) 


<div align="center">

![](https://github.com/liushengxi13689209566/MyLibCo/blob/master/image/LOGO.png)


A C++ network library for high concurrent server in Linux  by Coroutine

一个基于 Reactor 模式，使用协程开发的多进程/线程-协程模式工作的 C++ 网络库

![](https://img.shields.io/badge/release-v1.0-blue.svg)
![](https://img.shields.io/badge/build-passing-green.svg)
![](https://img.shields.io/badge/dependencies-up%20to%20date-green.svg)
![](https://img.shields.io/badge/license-MIT-blue.svg)

</div>

-----

##### 快速使用指南:

```cpp

//create a server 
//it create 2 process and every process create 2 routines
TcpServer server(2,2);

//set ConnetionCallback
server.setConnectionCallback(onConnection);

//set MessageCallback
server.setMessageCallback(onMessage);

//start the server
server.start();

```
##### 运行结果:


-----

## 特性:

- 网络模块采用 muduo 单线程的 Reactor 方案。
- 使用 timerfd 实现高性能时间堆定时器，并将其统一到 EventLoop 中。
- 可选的共享栈模式，单机轻松接入千万连接。
- 使用 汇编 实现任务之间的切换，保存上下文信息的功能。
- 无需侵入业务逻辑，把多进程、多线程服务改造成协程服务，并发能力得到巨大提升。
- 使用 C++ 语言开发，屏蔽了底层细节，方便用户编写服务器程序．

## 安装与使用:

```
    $ git clone git@github.com:liushengxi13689209566/MyLibCo.git 

    $ cd MyLibCo/src

    $ make 

    $ ./echo_main 
    
```

## 具体使用方法:

```cpp
#include <iostream>
#include <unistd.h>
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
	printf("main(): pid = %d\n", getpid());
	
    int port = 9981;
	char *ip = "127.0.0.1";
	
	if (argc > 1)
	{
		ip = argv[1];
		port = atoi(argv[2]);
	}
	/*
        创建一个　server，里面包含2个进程，每个进程中又拥有10个协程
    */
	TcpServer server(ip, port, 2, 10);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.start();
}

```
这里放一个　gif 的图片

![运行结果](https://github.com/liushengxi13689209566/MyLibCo/blob/master/image/frist.png)



## 运行环境:
    - OS: Linux
    - 

## 系列分析文章: