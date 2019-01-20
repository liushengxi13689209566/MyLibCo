/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 21时49分55秒
 ************************************************************************/

#include "ServerSocket.h"

using namespace Tattoo;

int main(void)
{
	int so = socket(AF_INET, SOCK_STREAM, 0);
	ServerSocket ser(so);
}
