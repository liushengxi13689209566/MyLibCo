// /*************************************************************************
//     > File Name: Hello.cpp
//     > Author: Liu Shengxi
//     > Mail: 13689209566@163.com
//     > Created Time: 2018年12月10日 星期一 17时26分26秒
//  ************************************************************************/

// #include "routine.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <sys/time.h>
// #include <stack>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <sys/un.h>
// #include <fcntl.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>
// #include <sys/wait.h>
// #include <iostream>
// using namespace std;

// static void *A(void *arg)
// {
//     printf("1 ");
//     co_yield_ct(); // 切出到主协程
//     printf("2 ");
// }

// static void *B(void *arg)
// {
//     using namespace std;
//     printf("x ");
//     co_yield_ct(); // 切出到主协程
//     printf("y ");
// }
// int main(void)
// {
//     stCoRoutine_t *coa, *cob;
//     co_create(&coa, NULL, A, NULL); //**stCoRoutine_t, *attr(shareMem),pfn,*arg
//     co_create(&cob, NULL, B, NULL);
//     co_resume(coa);
//     co_resume(cob);
//     co_resume(coa);
//     co_resume(cob);
//     printf("\n");
// }
