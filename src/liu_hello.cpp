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
    printf("1 ");
    // co_yield_ct(); // 切出到主协程
    coa->Yield();
    printf("2 ");
}

static void *B(void *arg)
{
    using namespace std;
    printf("x ");
    // co_yield_ct(); // 切出到主协程
    cob->Yield();
    printf("y ");
}

int main(void)
{
    coa = new Routine_t(get_curr_thread_env(), NULL, A, NULL);
    cob = new Routine_t(get_curr_thread_env(), NULL, B, NULL);

    coa->Resume();
    cob->Resume();
    coa->Resume();
    cob->Resume();

    // delete coa;
    // delete cob;

    printf("\n");
}
