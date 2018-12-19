/*************************************************************************
	> File Name: test2.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月17日 星期一 19时39分42秒
 ************************************************************************/

#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	ucontext_t context;

	getcontext(&context); //当前保存上下文
	puts("Hello world");
	sleep(1);
	setcontext(&context); //此函数将控制转移到上下文中ucp。执行从存储上下文的位置继续ucp
	return 0;
}
