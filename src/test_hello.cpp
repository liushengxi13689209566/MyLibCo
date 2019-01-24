/*************************************************************************
    > File Name: Hello.cpp
    > Author: Liu Shengxi
    > Mail: 13689209566@163.com
    > Created Time: 2018年12月10日 星期一 17时26分26秒
 ************************************************************************/

#include "routine.h"
#include "routine.cpp"

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
using namespace Tattoo;

static void *A(void *arg);
static void *B(void *arg);

Routine_t *coa;
Routine_t *cob;

static void *A(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "coroutine : " << *(int *)arg + i << std::endl;
        coa->Yield();
    }
    return NULL;
}

static void *B(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        std::cout << "coroutine : " << *(int *)arg + i << std::endl;
        cob->Yield();
    }
    return NULL;
}

int main(void)
{
    int t1 = 1;
    int t2 = 4;
    coa = new Routine_t(get_curr_thread_env(), NULL, A, &t1);
    cob = new Routine_t(get_curr_thread_env(), NULL, B, &t2);
    for (int i = 0; i < 10; i++)
    {
        coa->Resume();
        cob->Resume();
        std::cout << std::endl;
    }

    delete coa;
    delete cob;

    printf("\n");
}
