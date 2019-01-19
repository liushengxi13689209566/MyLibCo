/*************************************************************************
	> File Name: liu_routine.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: Wed 16 Jan 2019 12:15:28 PM CST
 ************************************************************************/

#ifndef _ROUTINE_H
#define _ROUTINE_H
#include <iostream>
#include "coctx.h"
#include "callback.h"

namespace Tattoo
{

//前向声明
class RoutineEnv_t;
class RoutineAttr_t;
class StackMemory_t;
class ShareStack_t;

class Routine_t
{
public:
  Routine_t(RoutineEnv_t *env, const RoutineAttr_t *attr, void *(*routine)(void *), void *arg);

  Routine_t(const Routine_t &Routine_t) noexcept = delete;
  Routine_t(Routine_t &&Routine_t) noexcept = delete;
  Routine_t &operator=(const Routine_t &Routine_t) = delete;
  Routine_t &operator=(Routine_t &&Routine_t) noexcept = delete;

  ~Routine_t() {}
  void Resume();
  void Yield();

  RoutineEnv_t *env_; //当前协程环境
  RoutineFun pfn_;    //协程回调函数
  void *arg_;         //函数参数
  Coctx_t ctx_;       //保存上下文

  bool IsRun_;         //协程是否运行 cstart
  bool IsDead_;        //协程是否结束 cend
  bool IsMainRoutine_; //是否是主协程 cIsMain
  /********************************************占坑，hook层***********************************************************/
  char EnableSysHook;
  char IsShareStack;

  StackMemory_t *stack_mem_;

  char *stack_sp_;         //顶指针
  unsigned int save_size_; //buff大小
  char *save_buffer_;      //buff
};

class RoutineEnv_t
{
public:
  RoutineEnv_t() : CallStackSize_(0), pending_rou_(NULL), occupy_rou_(NULL) {}
  Routine_t *CallStack_[128]; //用于保存当前线程中的协程　　
  int CallStackSize_;         //相当于栈指针

  Routine_t *pending_rou_;
  Routine_t *occupy_rou_;
};
class StackMemory_t
{
public:
  StackMemory_t(unsigned int stack_size)
      : occupy_routine_(NULL),
        stack_size_(stack_size)
  {
    stack_buffer_ = (char *)malloc(stack_size);
    stack_bp_ = stack_buffer_ + stack_size;
  }
  Routine_t *occupy_routine_; //当前协程指针
  unsigned int stack_size_;   //栈大小
  char *stack_bp_;            //stack_buffer + stack_size
  char *stack_buffer_;        //栈空间
};
class RoutineAttr_t
{
public:
  RoutineAttr_t() : stack_size_(128 * 1024), share_stack_(NULL) {}
  int stack_size_;
  ShareStack_t *share_stack_;
};
class ShareStack_t
{
public:
  ShareStack_t(int count, int stack_size)
      : alloc_idx_(0),
        stack_size_(stack_size),
        count_(count)
  {
    stack_array_ = (StackMemory_t **)calloc(count, sizeof(StackMemory_t *));
    for (int i = 0; i < count; i++)
    {
      stack_array_[i] = new StackMemory_t(stack_size);
    }
  }

  unsigned int alloc_idx_;
  int stack_size_;
  int count_;
  StackMemory_t **stack_array_;
};
// 2.Routine_t

// 3. ShareStack_t

// error:debug

} // namespace Tattoo

#endif
