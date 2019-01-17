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
/********************tool function******************************/
Routine_t *GetCurrRoutine(RoutineEnv_t *env);

/********************tool function end......********************/

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
    self->IsMainRoutine_ = true; //设置为主协程

    Coctx_init(&self->ctx_);
    env->CallStack_[env->CallStackSize_++] = self;
}
//得到当前线程的协程环境
RoutineEnv_t *get_curr_thread_env()
{
    return ArrayEnvPerThread[GetTid()];
}
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
    StackMemory_t *stack_mem = NULL;
    if (at.share_stack_)
    {
        stack_mem = get_stack_form_share(at.stack_size_);
        at.stack_size_ = at.share_stack_->stack_size_;
    }
    else
    {
        stack_mem = new StackMemory_t(at.stack_size_);
    }
    stack_mem_ = stack_mem;
    ctx_.ss_sp = stack_mem->stack_buffer_;
    ctx_.ss_size = stack_mem->stack_size_;
    save_size_ = 0;
    save_buffer_ = NULL;
    std::cout << "创建一个协程　" << std::endl;
}
void Routine_t::Resume()
{
    Routine_t *curr_routine = env_->CallStack_[env_->CallStackSize_ - 1];
    if (!IsRun_)
    {
        Coctx_make(&ctx_, (coctx_pfn_t)RoutineFunc, this, 0);
        IsRun_ = true;
    }
    env_->CallStack_[env_->CallStackSize_++] = this;
    Swap_two_routine(curr_routine, this);
}
} // namespace Tattoo
