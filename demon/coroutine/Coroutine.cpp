/*************************************************************************
	> File Name: Coroutine.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时01分04秒
 ************************************************************************/

#include <ucontext.h>
#include <memory>
#include <assert.h>
#include <string.h>

#include "Coroutine.h"

namespace Tattoo
{
Coroutine::Coroutine(CoroutineSchedule *sch, CoFun func, int id)
	: func_(func), id_(id), sch_(sch),
	  stack_max_size_(0), stack_cur_size_(0),
	  status_(CO_READY), stack_(0)
{
}
Coroutine::~Coroutine()
{
	delete[] stack_;
}

void Coroutine::SetStatus(int status)
{
	status_ = status;
}
void Coroutine::save_stack(void *top)
{
	char dummy = 0;

	assert((char *)top - &dummy <= CoroutineSchedule::STACK_SIZE);

	if (stack_max_size_ < (char *)top - &dummy)
	{
		delete[] stack_;
		stack_max_size_ = (char *)top - &dummy;
		stack_ = new char[stack_max_size_];
	}
	stack_cur_size_ = (char *)top - &dummy;
	memcpy(stack_, &dummy, stack_cur_size_);
}
} // namespace Tattoo
