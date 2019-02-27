
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
- 
- 
- 
- 
- 

## 运行环境:
    - OS: Linux
    - 

## 系列分析文章:


## 使用方法:

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

	//2  进程　
	//10 协程
	TcpServer server(ip, port, 2, 10);
	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	server.start();
}
```
![运行结果](https://github.com/liushengxi13689209566/MyLibCo/blob/master/image/frist.png)

PS:明天会完善　readme


