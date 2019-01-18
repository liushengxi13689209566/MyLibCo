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
/********************tool function******************************/
//                   about Routine_t

/*挂起当前的co_routine，切换到上一个co_routine，将当前的co_routine设置为上一个co_routine*/
void yield_env(RoutineEnv_t *env);

/*协程中要执行的函数,该函数会调用回调函数,并退出当前协程*/
static int RoutineFunc(Routine_t *rou, void *);

/*交换两个协程*/
void Swap_two_routine(Routine_t *curr, Routine_t *pending_rou);

/*将共享栈上的数据 copy out */
void save_stack_buffer(Routine_t *occupy_rou);

/*从共享栈区得到一个栈空间*/
static StackMemory_t *get_stack_form_share(ShareStack_t *share_stack);
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

    Coctx_init(&self->ctx_);
    env->CallStack_[env->CallStackSize_++] = self;
}
//得到当前线程的协程环境
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
        stack_mem = get_stack_form_share(at.share_stack_);
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

/*从共享栈区得到一个栈空间*/
static StackMemory_t *get_stack_form_share(ShareStack_t *share_stack)
{
    if (!share_stack)
    {
        return NULL;
    }
    int idx = share_stack->alloc_idx_ % share_stack->count;
    share_stack->alloc_idx_++;

    return share_stack->stack_array_[idx];
}

/*将共享栈上的数据 copy out */
void save_stack_buffer(Routine_t *occupy_rou)
{
    StackMemory_t *stack_mem = occupy_rou->stack_mem_;
    int len = stack_mem->stack_bp_ - occupy_rou->stack_sp_;
    //如果已经保存过一次
    if (occupy_rou->save_buffer_)
    {
        free(occupy_rou->save_buffer_);
        occupy_rou->save_buffer_ = NULL;
    }
    occupy_rou->save_buffer_ = new char(len);
    occupy_rou->save_size_ = len;
    memcpy(occupy_rou->save_buffer_, occupy_rou->stack_sp_, len);
}

/*交换两个协程*/
void Swap_two_routine(Routine_t *curr, Routine_t *pending_rou)
{
    RoutineEnv_t *env = get_curr_thread_env();
    char ch;
    //co_swap函数里面最后一个声明的局部变量，
    //ch所在的内存地址就是当前栈顶地址，即ESP寄存器内保存的值
    curr->stack_sp_ = &ch;
    if (!pending_rou->IsShareStack)
    {
        env->pending_rou_ = NULL;
        env->occupy_rou_ = NULL;
    }
    else //共享栈的情况下
    {
        env->pending_rou_ = pending_rou;
        //取出原本占用这块空间的corountine
        Routine_t *occupy_rou = pending_rou->stack_mem_->occupy_routine_;
        //将这块空间的占有者设置成将要运行的 coroutine
        pending_rou->stack_mem_->occupy_routine_ = pending_rou;

        env->occupy_rou_ = occupy_rou;
        //如果pending_co的栈区内存又被一个co_routine占用，
        //并且该co_routine不是pending_co，
        //则新申请一段内存区域保存下ocupy_co的stack_mem
        if (occupy_rou && occupy_rou != pending_rou)
        {
            save_stack_buffer(occupy_rou);
        }
    }
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
} // namespace Tattoo
/*+++++++++++++++++++++++++++各个函数实现 end..........++++++++++++++++++++++*/
