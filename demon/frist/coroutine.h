/*************************************************************************
	> File Name: coroutine.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月18日 星期二 16时32分11秒
 ************************************************************************/

#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <ucontext.h>

const int STACK_SIZE = 1024 * 1024;
const int DEFAULT_COROUTINE = 16;

class Coroutine
{
public:
	Coroutine()
	{
	}
};
class CoroutineScheduler
{
public:
	CoroutineScheduler()
	{
		class Coroutine *S = malloc(sizeof(*S));
		S->nco = 0;
		S->cap = DEFAULT_COROUTINE;
		S->running = -1;
		S->co = malloc(sizeof(struct coroutine *) * S->cap);
		memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
	}

	CoroutineScheduler(const CoroutineScheduler &) = delete;
	CoroutineScheduler &operator=(const CoroutineScheduler &) = delete;

private:
	char stack[STACK_SIZE];
	ucontext_t ctx_main;
	int nco;
	int cap;
	int running;
	// std::shared_ptr<std::shared_ptr<Coroutine>> co = nullptr;
	class coroutine **co;
};
#endif
