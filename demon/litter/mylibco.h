/*************************************************************************
	> File Name: mylibco.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月17日 星期一 21时03分55秒
 ************************************************************************/

#ifndef _MYLIBCO_HPP
#define _MYLIBCO_HPP
#include <functional>
#include <memory>
#include <map>
#include <ucontext.h>
#include <assert.h>

typedef std::function<void(void *)> Fun;

enum Status
{
	CO_READY,
	CO_SUSPENDED,
	CO_RUNNING,
	CO_FINISHED
};

struct coroutine
{
	// void *arg;
  public:
	coroutine(int size)
	{
	}
	int status;
	ucontext_t ctx;
	Fun func;
	char stack[1024];
};

class SchedulerImpl
{
  public:
	explicit SchedulerImpl(int stacksize) : index_(0), runing_(-1), stacksize_(stacksize) {}
	~SchedulerImpl() {}

	SchedulerImpl(const SchedulerImpl &) = delete;
	SchedulerImpl &operator=(const SchedulerImpl &) = delete;

	int CreateCoroutine(Fun func)
	{
		printf("start ....... \n");

		std::shared_ptr<coroutine> cor = std::make_shared<coroutine>(sizeof(coroutine) + stacksize_);

		printf("impossible  ....... \n");

		if (!cor)
			return -1;
		cor->func = func;
		cor->status = CO_READY;

		int index = index_++;
		id2routine_[index] = cor;
		return index;
	}
	int DestroyCoroutine(int id)
	{
		std::shared_ptr<coroutine> cor = id2routine_[id];
		if (!cor)
			return -1;
		id2routine_.erase(id);
		return id;
	}

	void Yield()
	{
		if (runing_ < 0)
			return;
		int cur = runing_;
		runing_ = -1;
		std::shared_ptr<coroutine> cor = id2routine_[cur];
		cor->status = CO_SUSPENDED;
		swapcontext(&cor->ctx, &mainContext_); //切换到 mainContext_ 上下文，保存当前上下文到 cor->ctx
	}

	void ResumeCoroutine(int id)
	{
		std::shared_ptr<coroutine> cor = id2routine_[id];
		if (!cor || cor->status == CO_RUNNING)
			return;
		switch (cor->status)
		{
		case CO_READY:
		{
			getcontext(&cor->ctx); //保存当前上下文
			cor->status = CO_RUNNING;
			cor->ctx.uc_stack.ss_sp = cor->stack;
			cor->ctx.uc_stack.ss_size = stacksize_;
			cor->ctx.uc_link = &mainContext_;
			runing_ = id;

			makecontext(&cor->ctx, (void (*)())Schedule, 1, this); //cor->ctx 的上下文指向 Schedule 函数
			swapcontext(&mainContext_, &cor->ctx);				   //切换到 ctx 上下文，保存当前上下文到 mainContext_
		}
		break;
		case CO_SUSPENDED:
		{
			runing_ = id;
			cor->status = CO_RUNNING;
			swapcontext(&mainContext_, &cor->ctx); //切换到 ctx 上下文，保存当前上下文到 mainContext_
		}
		break;
		default:
		{
		}
		}
		if (runing_ == -1 && cor->status == CO_FINISHED)
			DestroyCoroutine(id);
	}

	bool IsCoroutineAlive(int id) const
	{
		auto it = id2routine_.find(id);
		if (it != id2routine_.end())
			return true;
		else
			return false;
	}

  private:
	static void Schedule(void *arg) /*待填充*/
	{
	}

	int index_;
	int runing_;
	const int stacksize_;
	ucontext_t mainContext_;
	std::map<int, std::shared_ptr<coroutine>> id2routine_;
};

class CoroutineScheduler
{
  public:
	explicit CoroutineScheduler(int stacksize = 1024) {}
	~CoroutineScheduler() {}

	CoroutineScheduler(const CoroutineScheduler &) = delete;
	CoroutineScheduler &operator=(const CoroutineScheduler &) = delete;

	int DestoryCoroutine(int id)
	{
		return imp1->DestroyCoroutine(id);
	}
	int CreateCoroutine(Fun func)
	{
		return imp1->CreateCoroutine(func);
	}

	void Yield() /*切换*/
	{
		return imp1->Yield();
	}
	void ResumeCoroutine(int id) /*启用*/
	{
		return imp1->ResumeCoroutine(id);
	}

	bool IsCoroutineAlive(int id) const
	{
		return imp1->IsCoroutineAlive(id);
	}

  private:
	std::shared_ptr<SchedulerImpl> imp1 = nullptr;
};

#endif
