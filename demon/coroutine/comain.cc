#include "coroutine.h"

#include <iostream>

using namespace std;

CoroutineScheduler *sched = NULL;

void func1(void *arg)
{
    int test = *(int *)arg;
    cout << "function1 a now!,arg:" << test << ", start to yield." << endl;
    sched->Yield();
    cout << "1.fun1 return from yield  " << endl;
    sched->Yield();
    cout << "2.fun1 return from yield: going to stop" << endl;
}

void func2(void *s)
{
    cout << "function2 a now!, arg: start to yield." << endl;
    sched->Yield();
    cout << "fun2 return from yield:going to stop" << endl;
}

int main()
{
    sched = new CoroutineScheduler();

    bool stop = false;
    int f1 = sched->CreateCoroutine(func1, (void *)111);
    int f2 = sched->CreateCoroutine(func2, (void *)222);

    while (!stop)
    {
        stop = true;
        if (sched->IsCoroutineAlive(f1))
        {
            stop = false;
            sched->ResumeCoroutine(f1);
            cout << "func1 yield: " << endl;
        }

        if (sched->IsCoroutineAlive(f2))
        {
            stop = false;
            sched->ResumeCoroutine(f2);
            cout << "func2 yield:" << endl;
        }
    }

    delete sched;
    return 0;
}
