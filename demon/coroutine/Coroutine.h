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
#include "CoroutineSchedule.h"

namespace Tattoo
{
class Coroutine
{
  public:
	using CoFun = std::function<void()>;
	enum
	{
		CO_FINSHED,
		CO_READY,
		CO_RUNNING,
		CO_SUSPEND,
	};
	Coroutine(CoroutineSchedule *sch, CoFun func, int id);
	~Coroutine();

	Coroutine(const Coroutine &) = delete;
	Coroutine &operator=(const Coroutine &) = delete;

	void SetStatus(int status);
	int GetStatus();

  private:
	friend class CoroutineSchedule;

	void save_stack(void *top);

	CoFun func_;
	ucontext_t ctx_;
	std::shared_ptr<CoroutineSchedule> sch_;
	std::ptrdiff_t stack_max_size_; //stack最大 大小
	std::ptrdiff_t stack_cur_size_; //协程实际大小
	int status_;
	char *CorStack;
	int id_;
};
} // namespace Tattoo
#endif
