/*************************************************************************
	> File Name: routine.h
	> Author: 
	> Mail: 
	> Created Time: 2018年02月10日 星期六 23时40分57秒
 ************************************************************************/

#ifndef _ROUTINE_H
#define _ROUTINE_H
#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <string>
#include <functional>
#include "callback.h"
#include "coctx.h"
// #include "Time_heap.h"

namespace Tattoo
{

//向前声明
class RoutineEnv_t;
class StackMemory_t;
class RoutineAttr_t;

//协程块
class Routine_t
{
public:
  //FIXME RoutineAttr_t use the *
  Routine_t(RoutineEnv_t *env, const RoutineAttr_t *attr, void *(*routine)(void *), void *arg);

  Routine_t(const Routine_t &Routine_t) noexcept = delete;
  Routine_t(Routine_t &&Routine_t) noexcept = delete;
  Routine_t &operator=(const Routine_t &Routine_t) = delete;
  Routine_t &operator=(Routine_t &&Routine_t) noexcept = delete;

  ~Routine_t();
  void Resume(); //运行当前协程
  void Yield();  //退出当前协程
public:
  RoutineEnv_t *env_; //当前协程环境
  RoutineFun pfn_;    //协程块函数
  void *arg_;         //协程块函数对应的参数
  coctx_t ctx_;       //用来保存CPU上下文

  char IsRun_;         //协程是否运行   cstart
  char IsDead_;        //协程是否结束   cend
  char IsMainRoutine_; //是否是主协程   cIsMain

  /********************************************占坑，hook层***********************************************************/
  char EnableSysHook;
  char IsShareStack;

  StackMemory_t *stack_memory_; //协程运行栈内存

  char *stack_sp_;         //顶指针
  unsigned int save_size_; //buff大小
  char *save_buffer_;      //buff
};

class RoutineEnv_t
{
public:
  RoutineEnv_t() : CallStackSize_(0), pending_rou_(NULL), occupy_rou_(NULL) {}
  Routine_t *CallStack_[128]; //保存调用链
  int CallStackSize_;         //栈指针

  Routine_t *pending_rou_;
  Routine_t *occupy_rou_;

  //时间堆定时器
  // Time_heap *time_heap_;
};

//协程栈
class StackMemory_t
{
public:
  StackMemory_t(int stack_size)
      : occupy_routine_(NULL),
        stack_size_(stack_size)
  {
    stack_buffer_ = (char *)malloc(stack_size_);
    stack_bp_ = stack_buffer_ + stack_size_;
  }

public:
  Routine_t *occupy_routine_; //当前协程指针
  int stack_size_;            //栈大小
  char *stack_bp_;            //stack_buffer + stack_size
  char *stack_buffer_;        //栈空间
};

class RoutineAttr_t
{
public:
  RoutineAttr_t() : stack_size_(128 * 1024) {}
  int stack_size_;

  // ShareStack_t *share_stack_;
};

//------------协程调度-------------------------------

// class EventLoop
// {
// public:
//   EventLoop(Time_heap *time_heap, EventLoop_Function runInLoopFunction, void *arg);
//   ~EventLoop();
//   void loop();
//   void runInLoop();

// public:
//   EventLoop_Function runInLoopFunction_;
//   void *arg_;

//   //定时器指针
//   Time_heap *time_heap_;
// };

//--------------------------

} // namespace Tattoo

#endif
