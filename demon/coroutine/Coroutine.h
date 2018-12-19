/*************************************************************************
	> File Name: Coroutine.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时01分04秒
 ************************************************************************/

#ifndef _COROUTINE_H
#define _COROUTINE_H
#include <ucontext.h>
#include <memory>
#include <assert.h>
#include <string.h>
#include "./CoroutineSchedule.h"

namespace Tattoo
{
class Coroutine
{
  public:
	enum
	{
		CO_DEAD,
		CO_READY,
		CO_RUNNING,
		CO_SUSPEND,
	};
	Coroutine(CoroutineSchedule *sch, CoFun func, int id)
		: func_(func), id_(id), sch_(sch),
		  stack_max_size_(0), stack_cur_size_(0),
		  status_(CO_READY), stack_(0)
	{
	}
	~Coroutine()
	{
		delete[] stack_;
	}

	Coroutine(const Coroutine &) = delete;
	Coroutine &operator=(const Coroutine &) = delete;

	void SetStatus(int status)
	{
		status_ = status;
	}

  private:
	friend class CoroutineSchedule;

	void _save_stack(void *top)
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

	CoFun func_;
	ucontext_t ctx_;
	std::shared_ptr<CoroutineSchedule> sch_;
	std::ptrdiff_t stack_max_size_; //stack最大 大小
	std::ptrdiff_t stack_cur_size_; //协程实际大小
	int status_;
	char *stack_;
	int id_;
};
} // namespace Tattoo
#endif
