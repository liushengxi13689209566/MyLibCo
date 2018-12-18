

#include "mylibco.h"

#include <iostream>
using namespace std;

std::shared_ptr<CoroutineScheduler> sched = NULL;

void func1(void *arg)
{
    int test = *(int *)arg;
    cout << "function1  runing ^^^^^^^^^^^^^^^^^ " << test << endl;
    sched->Yield();
    cout << "function1  Yeiled one time  " << endl;

    sched->Yield();
    cout << "function  Yeiled two times##############  " << endl;
}
void func2(void *arg)
{
    int test = *(int *)arg;
    cout << "function2  now run " << test << endl;
    sched->Yield();
    cout << "function2 will stop ......" << endl;
}
int main(void)
{
    sched = std::make_shared<CoroutineScheduler>();

    bool stop = false;
    int f1 = sched->CreateCoroutine(std::bind(func1, reinterpret_cast<void *>(111)));
    int f2 = sched->CreateCoroutine(std::bind(func2, reinterpret_cast<void *>(222)));
    while (!stop)
    {
        stop = true;
        if (sched->IsCoroutineAlive(f1))
        {
            stop = false;
            sched->ResumeCoroutine(f1);
            cout << "func1  yeiled " << endl;
        }
        if (sched->IsCoroutineAlive(f2))
        {
            stop = false;
            sched->ResumeCoroutine(f2);
            cout << "func2  yeiled " << endl;
        }
    }
    return 0;
}
