/*************************************************************************
    > File Name: Hello.cpp
    > Author: Liu Shengxi 
    > Mail: 13689209566@163.com
    > Created Time: 2018年12月10日 星期一 17时26分26秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <iostream>
#include <memory>
#include "routine.h"

using namespace std;
using namespace Tattoo;

static void *A(void *arg)
{
    printf("1 ");
    Yield(); // 切出到主协程
    printf("2 ");
}

static void *B(void *arg)
{
    printf("x ");
    Yield(); // 切出到主协程
    printf("y ");
}
int main(void)
{
    std::shared_ptr<Routine> coa = std::make_shared<Routine>(NULL, A, NULL);
    std::shared_ptr<Routine> coa = std::make_shared<Routine>(NULL, B, NULL);

    // *Routine, 参数,协程函数,函数参数　*arg

    coa->resume();
    cob->resume();

    coa->resume();
    cob->resume();

    printf("\n");
}
