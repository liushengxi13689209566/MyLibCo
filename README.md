# MyLibCo
A C++ network library for high concurrent server in Linux by libco .

-----

## 简介:



## 运行环境:
    - OS: Linux
    - 

## 技术点:
    - 
    - 
    - 

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
![运行结果](https://github.com/liushengxi13689209566/MyLibCo/image/frist.png)
