#include "coroutine.h"
#include <stdio.h>

struct args
{
    int n;
};

CoroutineScheduler scheduler;

static void
foo(void *ud)
{
    struct args *arg = ud;
    int start = arg->n;
    int i;
    for (i = 0; i < 5; i++)
    {
        printf("coroutine %d : %d\n", schedule.GetId(), start + i);
        schedule.Yield();
    }
}

int main()
{
    struct args arg1 = {0};
    struct args arg2 = {100};

    // int co1 = coroutine_new(S, foo, &arg1);

    int co1 = schedule.CreateCoroutine(foo, &arg1);
    int co2 = schedule.CreateCoroutine(foo, &arg2);

    // int co2 = coroutine_new(S, foo, &arg2);

    printf("main start\n");

    // while (coroutine_status(S, co1) && coroutine_status(S, co2))
    // {
    //     coroutine_resume(S, co1);

    //     coroutine_resume(S, co2);
    // }
    while (1)
    {
        if (schedule.IsAlive(co1))
            schedule.ResumeCoroutine(co1);

        if (schedule.IsAlive(co2))
            schedule.ResumeCoroutine(co2);
    }
    printf("main end\n");

    return 0;
}
