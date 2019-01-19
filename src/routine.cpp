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
    extern void coctx_swap(coctx_t *, coctx_t *) asm("coctx_swap");
};
/********************tool function******************************/
//                   about Routine_t

/*挂起当前的co_routine，切换到上一个co_routine，将当前的co_routine设置为上一个co_routine*/
void yield_env(RoutineEnv_t *env);

/*协程中要执行的函数,该函数会调用回调函数,并退出当前协程*/
static int RoutineFunc(Routine_t *rou, void *);

/*交换两个协程*/
void Swap_two_routine(Routine_t *curr, Routine_t *pending_rou);

/********************tool function end......********************/

//得到 线程ID
static pid_t
GetTid()
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

    coctx_init(&self->ctx_);
    env->CallStack_[env->CallStackSize_++] = self;

    // pid_t pid = GetTid();
    // ArrayEnvPerThread[pid] = new RoutineEnv_t();
    // RoutineEnv_t *env = ArrayEnvPerThread[pid];

    // env->CallStackSize_ = 0;
    // Routine_t *self = new Routine_t(env, NULL, NULL, NULL);
    // self->IsMainRoutine_ = 1;

    // env->occupy_rou_ = NULL;
    // env->pending_rou_ = NULL;

    // coctx_init(&self->ctx_);

    // env->CallStack_[env->CallStackSize_++] = self;
}
//得到当前线程的协程环境
RoutineEnv_t *get_curr_thread_env()
{
    if (!ArrayEnvPerThread[GetTid()])
    {
        init_curr_thread_env();
    }
    //std::cout<<"GetTid : "<<GetTid()<<std::endl;
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
    // at = *attr;
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
    ctx_.ss_sp = stack_mem_->stack_buffer_;
    ctx_.ss_size = at.stack_size_;

    save_size_ = 0;
    save_buffer_ = NULL;
    std::cout << "创建一个协程　" << std::endl;
}
Routine_t::~Routine_t()
{

    //std::cout<<" Free Routine " << std::endl;
    free(stack_mem_->stack_buffer_);
    free(stack_mem_);
}
void Routine_t::Resume()
{
    RoutineEnv_t *env = env_;
    Routine_t *curr_routine = env->CallStack_[env_->CallStackSize_ - 1];
    if (!IsRun_)
    {
        coctx_make(&ctx_, (coctx_pfn_t)RoutineFunc, this, 0);
        IsRun_ = true;
    }
    env->CallStack_[env_->CallStackSize_++] = this;
    Swap_two_routine(curr_routine, this);
}
void Routine_t::Yield()
{
    yield_env(env_);
}

/*+++++++++++++++++++++++++++各个函数实现+++++++++++++++++++++++++++++++++++*/
/*挂起当前的co_routine，切换到上一个co_routine，将当前的co_routine设置为上一个co_routine*/
void yield_env(RoutineEnv_t *env)
{
    Routine_t *last = env->CallStack_[env->CallStackSize_ - 2];
    Routine_t *curr = env->CallStack_[env->CallStackSize_ - 1];
    env->CallStackSize_--;
    Swap_two_routine(curr, last);
}
/*协程中要执行的函数,该函数会调用回调函数,并退出当前协程*/
static int RoutineFunc(Routine_t *rou, void *)
{
    if (rou->pfn_)
    {
        rou->pfn_(rou->arg_);
    }
    rou->IsDead_ = true;
    RoutineEnv_t *env = rou->env_;
    yield_env(env);
    return 0;
}

/*交换两个协程*/
void Swap_two_routine(Routine_t *Current_Routine, Routine_t *Pending_Routine)
{
    // RoutineEnv_t *env = get_curr_thread_env();
    // char ch;
    // //co_swap函数里面最后一个声明的局部变量，
    // //ch所在的内存地址就是当前栈顶地址，即ESP寄存器内保存的值
    // curr->stack_sp_ = &ch;

    // env->pending_rou_ = NULL;
    // env->occupy_rou_ = NULL;

    // //这句代码执行完成后，CPU已经切换到 pending_co
    // coctx_swap(&(curr->ctx_), &(pending_rou->ctx_));
    // //pending_co 退出，又回到 curr

    // RoutineEnv_t *curr_env = get_curr_thread_env();
    // Routine_t *update_occupy_co = curr_env->occupy_rou_;
    // Routine_t *update_pending_co = curr_env->pending_rou_;

    // if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co)
    // {
    //     // resume stack buffer
    //     if (update_pending_co->save_buffer_ && update_pending_co->save_size_ > 0)
    //     {
    //         memcpy(update_pending_co->stack_sp_, update_pending_co->save_buffer_, update_pending_co->save_size_);
    //     }
    // }
    RoutineEnv_t *env = get_curr_thread_env();

    //得到当前栈指针
    //get curr stack sp
    char c;
    Current_Routine->stack_sp_ = &c;

    env->pending_rou_ = NULL;
    env->occupy_rou_ = NULL;

    //swap context
    coctx_swap(&(Current_Routine->ctx_), &(Pending_Routine->ctx_));

    //防止栈内存被修改
    //重新得到栈内存
    //stack buffer may be overwrite, so get again;
    RoutineEnv_t *curr_env = get_curr_thread_env();
    Routine_t *update_occupy_co = curr_env->occupy_rou_;
    Routine_t *update_pending_co = curr_env->pending_rou_;
    if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co)
    {
        //resume stack buffer
        if (update_pending_co->save_buffer_ && update_pending_co->save_size_ > 0)
        {
            memcpy(update_pending_co->stack_sp_, update_pending_co->save_buffer_,
                   update_pending_co->save_size_);
        }
    }
}
} // namespace Tattoo
/*+++++++++++++++++++++++++++各个函数实现 end..........++++++++++++++++++++++*/
