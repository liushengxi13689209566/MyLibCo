/*************************************************************************
	> File Name: routine.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月23日 星期日 17时04分45秒
 ************************************************************************/

#ifndef _ROUTINE_H
#define _ROUTINE_H
#include "./head.h"

namespace Tattoo
{
class RoEnv;
class RoutineAttr;
class StackMemry;

class Routine
{
	Routine();
	~Routine();

  public:
	RoEnv *env_;		//目前的协程环境
	RoFunction co_fun_; //协程函数
	void *arg_;			//协程函数的参数
	coctx_t ctx_;		//保存上下文

	char Start_;		 //协程是否运行
	char End_;			 //协程是否结束
	char IsMainRoutine_; //是否是主协程

	StackMemry *stack_memry_; //协程运行栈

	char *stack_sp_;		 //栈顶指针
	unsigned int save_size_; //buff 大小
	char *save_buffer_;		 //buffer

	/*下面可以添加协程私有数据*/
};
class RoEnv
{
	Routine *CallStack_[128]; //调用链
	int CallStackSize_;		  //链大小

	//for copy stack log lastco and nextco
	Routine *pending_co_;
	Routine *occupy_co_;
};
/*协程参数,就是指定运行栈的大小*/
class RoutineAttr
{
  public:
	RoutineAttr()
	{
		stack_size = 128 * 1024;
	}
	int stack_size;
};
/*真正的使用到的协程栈*/
class StackMemry()
{
  public:
	StackMemry(int stack_size) : occupy_co_(NULL), stack_size_(stack_size)
	{
		stack_buffer_ = new char[stack_size_];
	}

	Routine *occupy_co_; //当前协程指针
	int stack_size_;	 //栈大小
	char *stack_bp_;	 //栈底　
	char *stack_buffer_; //运行栈
};
} // namespace Tattoo
#endif
