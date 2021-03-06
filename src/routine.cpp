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
#include "routine.h"

namespace Tattoo
{
extern "C"
{
    extern void coctx_swap(Coctx_t *, Coctx_t *) asm("coctx_swap");
};

/************************** copy in || copy out ****************************************/

/************************** swap two routine ****************************************/
void Swap_two_routine(Routine_t *curr, Routine_t *pending_rou)
{
    RoutineEnv_t *env = get_curr_thread_env();
    char ch;
    //co_swap函数里面最后一个声明的局部变量，
    //ch所在的内存地址就是当前栈顶地址，即ESP寄存器内保存的值
    curr->stack_sp_ = &ch;
    env->pending_rou_ = NULL;
    env->occupy_rou_ = NULL;

    //这句代码执行完成后，CPU已经切换到pending_co
    coctx_swap(&(curr->ctx_), &(pending_rou->ctx_));
    //pending_co 退出，又回到 curr
    RoutineEnv_t *curr_env = get_curr_thread_env();
    Routine_t *update_occupy_co = curr_env->occupy_rou_;
    Routine_t *update_pending_co = curr_env->pending_rou_;

    if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co)
    {
        // resume stack buffer
        if (update_pending_co->save_buffer_ && update_pending_co->save_size_ > 0)
        {
            memcpy(update_pending_co->stack_sp_, update_pending_co->save_buffer_, update_pending_co->save_size_);
        }
    }
}

/************************** about Routine ********************************************/
void init_curr_thread_env()
{
    pid_t tid = GetTid();
    ArrayEnvPerThread[tid] = new RoutineEnv_t();
    RoutineEnv_t *env = ArrayEnvPerThread[tid];

    Routine_t *self = new Routine_t(env, NULL, NULL, NULL);
    self->IsMainRoutine_ = true; //设置为主协程

    Coctx_init(&self->ctx_);
    env->CallStack_[env->CallStackSize_++] = self;

    // env->time_heap_ = new MiniHeap();
    // env->epoll_ = new Epoll();
}
RoutineEnv_t *get_curr_thread_env()
{
    if (!ArrayEnvPerThread[GetTid()])
    {
        init_curr_thread_env();
    }
    //std::cout<<"GetPid : "<<GetPid()<<std::endl;
    return ArrayEnvPerThread[GetTid()];
}

Routine_t::Routine_t(RoutineEnv_t *env, const RoutineAttr_t *attr,
                     RoutineFun pfn, void *arg)
    : env_(env),
      pfn_(pfn),
      arg_(arg),
      start_(0),
      end_(0),
      IsMainRoutine_(0),
      EnableSysHook_(0)
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
    StackMemory_t *stack_mem = new StackMemory_t(at.stack_size_);

    stack_mem_ = stack_mem;
    ctx_.ss_sp = stack_mem->stack_buffer_;
    ctx_.ss_size = stack_mem->stack_size_;
    // IsShareStack_ = at.share_stack_ != NULL;

    save_size_ = 0;
    save_buffer_ = NULL;
    // std::cout << "创建一个协程　" << std::endl;
}
void Routine_t::Resume()
{
    Routine_t *curr_routine = env_->CallStack_[env_->CallStackSize_ - 1];
    if (!start_)
    {
        Coctx_make(&ctx_, (coctx_pfn_t)RoutineFunc, this, 0);
        start_ = 1;
    }
    env_->CallStack_[env_->CallStackSize_++] = this;
    Swap_two_routine(curr_routine, this);
}
void Routine_t::Yield()
{
    yield_env(env_);
}

void yield_env(RoutineEnv_t *env)
{
    Routine_t *last = env->CallStack_[env->CallStackSize_ - 2];
    Routine_t *curr = env->CallStack_[env->CallStackSize_ - 1];
    env->CallStackSize_--;
    Swap_two_routine(curr, last);
}
/************************** other ********************************************/
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
static int RoutineFunc(Routine_t *rou, void *)
{
    if (rou->pfn_)
    {
        rou->pfn_(rou->arg_);
    }
    rou->end_ = 1;
    RoutineEnv_t *env = rou->env_;
    yield_env(env);
    return 0;
}
Routine_t *get_curr_routine()
{
    // std::cout << "get_curr_routine" << std::endl;
    RoutineEnv_t *env = get_curr_thread_env();
    return env->CallStack_[env->CallStackSize_ - 1];
}
} // namespace Tattoo
