/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月20日 星期四 09时32分16秒
 ************************************************************************/

#include <iostream>
using namespace std;
//确定栈的生长方向
//自定义函数fun1() 和fun2()，其中fun1()内部调用fun2()，输出参数地址
#include <stdio.h>

void fun2(int b)
{
	printf("fun2: %ld\n", &b);
}

void fun1(int a)
{
	printf("fun1: %ld\n", &a);
	fun2(a);
}
int main()
{

	int a = 1;
	fun1(a);
	char str[1024 * 1024];
	printf("str: %p\n", &str);
	printf("stt+SIZE: %p\n", &str + 1024 * 1024);

	return 0;
}
