/*************************************************************************
	> File Name: routine.cc
	> Author: 
	> Mail: 
	> Created Time: 2018年02月11日 星期日 00时56分37秒
 ************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <map>
#include <poll.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <limits.h>
#include <memory>

#include "routine.h"
#include "Time_heap.h"
//#include"routine_inner.h"
#include <functional>
namespace Tattoo
{
extern "C"
{
    extern void coctx_swap(coctx_t *, coctx_t *) asm("coctx_swap");
};

class Time_heap;

//-------debug---------
void dbg_flag(int line)
{
    std::cout << "I'm Runing on " << line << std::endl;
}
void dbg_print(char *str)
{
    std::cout << str << std::endl;
}

//----------------Function Definition-----------------------------
static int RoutineFunc(Routine_t *routine, void *);
void yield_env(RoutineEnv_t *env_);
void co_swap(Routine_t *Current_Routine, Routine_t *Pending_Routine);
void init_curr_thread_env();
RoutineEnv_t *get_curr_thread_env();

//---------------------toolsfunction------------------------------
//获取时间

//得到当前ID
static pid_t GetPid()
{
    static __thread pid_t pid = 0;
    static __thread pid_t tid = 0;
    if (!pid || !tid || pid != getpid())
    {
        tid = syscall(__NR_gettid);
    }
    return tid;
}

//-------------------------------------------------

//存储每个线程的协程状态
static RoutineEnv_t *g_arryEnvPerThread[204800] = {0};

//-------------RoutineEnv_t--------------------------

//初始化当前线程的协程环境
//并初始化主协程
void init_curr_thread_env()
{
    pid_t pid = GetPid();
    g_arryEnvPerThread[pid] = new RoutineEnv_t();
    RoutineEnv_t *env = g_arryEnvPerThread[pid];

    env->CallStackSize_ = 0;
    Routine_t *self = new Routine_t(env, NULL, NULL, NULL);
    self->IsMainRoutine_ = 1;

    env->pending_rou_ = NULL;
    env->occupy_rou_ = NULL;

    coctx_init(&self->ctx_);

    env->CallStack_[env->CallStackSize_++] = self;

    //初始化定时器
    // env->time_heap_ = new Time_heap(50);
    //stCoEpoll_t *ev = AllocEpoll();
    //SetEpoll( env,ev );
}

//得到当前线程的协程环境,如果没有,则进行初始化
RoutineEnv_t *get_curr_thread_env()
{
    if (!g_arryEnvPerThread[GetPid()])
    {
        init_curr_thread_env();
    }
    //std::cout<<"GetPid : "<<GetPid()<<std::endl;
    return g_arryEnvPerThread[GetPid()];
}

//----------------Routine-----------------
Routine_t::Routine_t(RoutineEnv_t *env,
                     const RoutineAttr_t *attr,
                     void *(*pfn)(void *), void *arg)
    : env_(env),
      pfn_(pfn),
      arg_(arg),
      IsRun_(0),
      IsDead_(0),
      IsMainRoutine_(0)
{
    RoutineAttr_t at;
    if (attr)
    {
        at = *attr;
    }

    //初始化栈大小
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

    //分配栈内存
    StackMemory_t *stack_memry = new StackMemory_t(at.stack_size_);

    stack_memory_ = stack_memry;

    ctx_.ss_sp = stack_memory_->stack_buffer_;
    ctx_.ss_size = at.stack_size_;

    save_size_ = 0;
    save_buffer_ = NULL;

    //----debug-----
    std::cout << "成功创建了一个协程!" << std::endl;
}

//执行本协程
void Routine_t::Resume()
{

    RoutineEnv_t *env = env_;
    Routine_t *Current_Routine = env->CallStack_[env->CallStackSize_ - 1]; //取当前协程块指针
    //std::cout<<env->CallStackSize_<<std::endl;
    //协程首次执行,创建其上下文
    if (!IsRun_)
    {
        coctx_make(&ctx_, (coctx_pfn_t)RoutineFunc, this, 0);
        IsRun_ = 1;
    }

    env->CallStack_[env->CallStackSize_++] = this; //将新协程块指针压入pCallStack栈中
    co_swap(Current_Routine, this);
}

//退出本协程
void Routine_t::Yield()
{
    //确保不重复退出协程
    assert(IsDead_ == 0 && IsRun_ == 1);
    yield_env(env_);
}

Routine_t::~Routine_t()
{

    //std::cout<<" Free Routine " << std::endl;
    free(stack_memory_->stack_buffer_);
    free(stack_memory_);
}

void yield_env(RoutineEnv_t *env_)
{
    Routine_t *Last_routine = env_->CallStack_[env_->CallStackSize_ - 2];    //获取上一个协程块
    Routine_t *Current_Routine = env_->CallStack_[env_->CallStackSize_ - 1]; //获取当前协程块

    env_->CallStackSize_--;
    co_swap(Current_Routine, Last_routine);
}

//-------------------------------------------

//协程中要执行的函数
//该函数会调用回调函数
//并退出当前协程
static int RoutineFunc(Routine_t *routine, void *)
{
    //执行协程函数
    if (routine->pfn_)
    {
        routine->pfn_(routine->arg_);
    }

    routine->IsDead_ = 1;

    RoutineEnv_t *env = routine->env_;
    //切回上一个栈中的上一个协程
    //while(1)防止被函数执行完后多次调用resume
    while (1)
        yield_env(env);
    return 0;
}

//交换执行两个协程
void co_swap(Routine_t *Current_Routine, Routine_t *Pending_Routine)
{
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
    Routine_t *update_occupy_rou_co = curr_env->occupy_rou_;
    Routine_t *update_pending_rou_co = curr_env->pending_rou_;
    if (update_occupy_rou_co && update_pending_rou_co && update_occupy_rou_co != update_pending_rou_co)
    {
        //resume stack buffer
        if (update_pending_rou_co->save_buffer_ && update_pending_rou_co->save_size_ > 0)
        {
            memcpy(update_pending_rou_co->stack_sp_, update_pending_rou_co->save_buffer_,
                   update_pending_rou_co->save_size_);
        }
    }
}

//------------协程调度-----------------

//参数1 : 事件定时器
//参数2 : 在EventLoop提供的回调函数
//参数3 : 回调函数的参数
// EventLoop::EventLoop(Time_heap *time_heap, EventLoop_Function runInLoopFunction, void *arg)
//     : time_heap_(time_heap),
//       runInLoopFunction_(runInLoopFunction),
//       arg_(arg)

// {
// }

// EventLoop::~EventLoop()
// {
// }

// //执行EventLoop要执行的函数
// void EventLoop::runInLoop()
// {
//     if (runInLoopFunction_)
//     {
//         runInLoopFunction_(arg_);
//     }
// }

// void EventLoop::loop()
// {
//     while (1)
//     {
//         time_heap_->runOutTimeEvent();
//         runInLoop();
//     }
// }

} // namespace Tattoo
