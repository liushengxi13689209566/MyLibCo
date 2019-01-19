#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "coctx.h"
#include "routine.h"
#include "routine.cpp"

using namespace Tattoo;

Routine_t *co[2];

void *RoutineFunc(void *args)
{
    // co_enable_hook_sys();
    int *routineid = (int *)args;
    while (true)
    {
        char sBuff[128];
        sprintf(sBuff, "from routineid %d stack addr %p\n", *routineid, sBuff);

        printf("%s", sBuff);
        // poll(NULL, 0, 1000); //sleep 1s
        // sleep(1);
        co[*routineid]->Yield();
    }
    return NULL;
}

int main()
{
    ShareStack_t *share_stack = new ShareStack_t(1, 1024 * 128);

    // ShareStack_t *share_stack = co_alloc_sharestack(1, 1024 * 128);
    RoutineAttr_t attr(0, share_stack);

    int routineid[2];
    for (int i = 0; i < 2; i++)
    {
        routineid[i] = i;
        co[i] = new Routine_t(get_curr_thread_env(), &attr, RoutineFunc, routineid + i);
    }
    // co_eventloop(co_get_epoll_ct(), NULL, NULL);
    while (true)
    {
        co[1]->Resume();
        sleep(1);
        co[0]->Resume();
    }
    return 0;
}
