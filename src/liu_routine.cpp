/*************************************************************************
	> File Name: liu_routine.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: Wed 16 Jan 2019 12:15:34 PM CST
 ************************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/syscall.h> /* For SYS_xxx definitions */
#include <cstring>
#include "liu_routine.h"
#include "liu_callback.h"

namespace Tattoo
{
extern "C"
{
    extern void coctx_swap(Coctx_t *, Coctx_t *) asm("coctx_swap");
};

Routine_t *GetCurrRoutine(RoutineEnv_t *env);

/*辅助函数*/
//获取时间
//得到 线程ID
static pid_t GetTid()
{
    static __thread pid_t pid = 0;
    static __thread pid_t tid = 0; //定义一个线程局部变量很简单
    if (!pid || !tid || pid != getpid())
    {
        pid = getpid();
        tid = syscall(__NR_gettid);
    }
    return tid;
}
//存储对应于每一个线程的 RoutineEnv_t 结构
static RoutineEnv_t *ArrayEnvPerThread[204800] = {0};

/**************************RoutineEnv_t ****************************************/
//初始化当前线程的协程环境
void init_curr_thread_env()
{
    pid_t tid = GetTid();
    ArrayEnvPerThread[tid] = new RoutineEnv_t();
    RoutineEnv_t *env = ArrayEnvPerThread[tid];

    Routine_t *self = new Routine_t(env, NULL, NULL, NULL);
    self->IsMainRoutine_ = true;

    Coctx_init(&self->ctx_);
    env->CallStack_[env->CallStackSize_++] = self;
}
//得到当前线程的协程环境，如果没有就进行初始化
RoutineEnv_t *get_curr_thread_env()
{
    return ArrayEnvPerThread[GetTid()];
}
// 得到当前　协程

Routine_t::Routine_t(RoutineEnv_t *env, const RoutineAttr_t *attr,
                     RoutineFun pfn, void *arg)
    : env_(env),
      pfn_(pfn),
      arg_(arg),
      IsRun_(false),
      IsDead_(false),
      IsMainRoutine_(false),
      EnableSysHook(0)
{
    RoutineAttr_t at;
    if (attr)
        memcpy(&at, attr, sizeof(at));
    if (at.stack_size_ <= 0)
    {
        at.stack_size_ = 128 * 1024;
    }
    else if (at.stack_size_ > 1024 * 1024 * 8)
    {
        at.stack_size_ = 1024 * 1024 * 8;
    }

    if (at.stack_size_ & 0xFFF)
    {
        at.stack_size_ &= ~0xFFF;
        at.stack_size_ += 0x1000;
    }
    //协程自己的栈内存
    StackMemory_t *stack_mem = new StackMemory_t();
    if (at.share_stack_)
    {
        
    }
    else
    {
    }
}

} // namespace Tattoo
