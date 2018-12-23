#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#define STACK_SIZE (1024 * 1024)
#define DEFAULT_COROUTINE 16

struct coroutine;

struct schedule
{
	char stack[STACK_SIZE];
	ucontext_t main;
	int nco;
	int cap;
	int running;
	struct coroutine **co;
};

struct coroutine
{
	coroutine_func func;
	void *ud;
	ucontext_t ctx;
	struct schedule *sch;
	ptrdiff_t cap;
	ptrdiff_t size;
	int status;
	char *stack;
};

struct coroutine *
_co_new(struct schedule *S, coroutine_func func, void *ud)
{
	struct coroutine *co = malloc(sizeof(*co));
	co->func = func;
	co->ud = ud;
	co->sch = S;
	co->cap = 0;
	co->size = 0;
	co->status = COROUTINE_READY;
	co->stack = NULL;
	return co;
}

void _co_delete(struct coroutine *co)
{
	free(co->stack);
	free(co);
}

struct schedule *
coroutine_open(void)
{
	struct schedule *S = malloc(sizeof(*S));
	S->nco = 0;
	S->cap = DEFAULT_COROUTINE;
	S->running = -1;
	S->co = malloc(sizeof(struct coroutine *) * S->cap);
	memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
	return S;
}

void coroutine_close(struct schedule *S)
{
	int i;
	for (i = 0; i < S->cap; i++)
	{
		struct coroutine *co = S->co[i];
		if (co)
		{
			_co_delete(co);
		}
	}
	free(S->co);
	S->co = NULL;
	free(S);
}

int coroutine_new(struct schedule *S, coroutine_func func, void *ud)
{
	struct coroutine *co = _co_new(S, func, ud);
	if (S->nco >= S->cap)
	{
		int id = S->cap;
		S->co = realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
		memset(S->co + S->cap, 0, sizeof(struct coroutine *) * S->cap);
		S->co[S->cap] = co;
		S->cap *= 2;
		++S->nco;
		return id;
	}
	else
	{
		int i;
		for (i = 0; i < S->cap; i++)
		{
			int id = (i + S->nco) % S->cap;
			if (S->co[id] == NULL)
			{
				S->co[id] = co;
				++S->nco;
				return id;
			}
		}
	}
	assert(0);
	return -1;
}

static void
mainfunc(uint32_t low32, uint32_t hi32)
{
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	struct schedule *S = (struct schedule *)ptr;
	int id = S->running;
	struct coroutine *C = S->co[id];
	C->func(S, C->ud);
	_co_delete(C);
	S->co[id] = NULL;
	--S->nco;
	S->running = -1;
}

void coroutine_resume(struct schedule *S, int id)
{
	assert(S->running == -1);
	assert(id >= 0 && id < S->cap);
	struct coroutine *C = S->co[id];
	if (C == NULL)
		return;
	int status = C->status;
	switch (status)
	{
	case COROUTINE_READY:
		getcontext(&C->ctx);
		C->ctx.uc_stack.ss_sp = S->stack;
		C->ctx.uc_stack.ss_size = STACK_SIZE;
		C->ctx.uc_link = &S->main;
		S->running = id;
		C->status = COROUTINE_RUNNING;
		uintptr_t ptr = (uintptr_t)S;
		makecontext(&C->ctx, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
		swapcontext(&S->main, &C->ctx);
		break;
	case COROUTINE_SUSPEND:
		printf("S->stack ==%p\n", S->stack);
		printf("S->stack+STACK_SIZE ==%p\n", S->stack + STACK_SIZE);
		printf("\n\nS->stack + STACK_SIZE - C->size==  %p \n\n", S->stack + STACK_SIZE - C->size);

		memcpy(S->stack + STACK_SIZE - C->size, C->stack, C->size);

		S->running = id;
		C->status = COROUTINE_RUNNING;
		swapcontext(&S->main, &C->ctx);
		break;
	default:
		assert(0);
	}
}

static void
_save_stack(struct coroutine *C, char *top)
{
	printf("top:%p \n", top);

	char dummy = 0;
	printf("dummy:%p \n", &dummy);

	assert(top - &dummy <= STACK_SIZE);

	printf("STACK_ZISE:%d \n", STACK_SIZE);
	printf("top - &dummy:%ld \n\n\n\n", top - &dummy);

	if (C->cap < top - &dummy)
	{
		free(C->stack);
		C->cap = top - &dummy;
		C->stack = malloc(C->cap);
	}
	C->size = top - &dummy; /*c->stack 是在堆上的*/
	memcpy(C->stack, &dummy, C->size);
}

void coroutine_yield(struct schedule *S)
{
	int id = S->running;
	assert(id >= 0);
	struct coroutine *C = S->co[id];
	assert((char *)&C > S->stack);

	printf("S->stack ==%p\n", S->stack);
	printf("S->stack+STACK_SIZE ==%p\n", S->stack + STACK_SIZE);

	_save_stack(C, S->stack + STACK_SIZE);

	C->status = COROUTINE_SUSPEND;
	S->running = -1;
	swapcontext(&C->ctx, &S->main);
}

int coroutine_status(struct schedule *S, int id)
{
	assert(id >= 0 && id < S->cap);
	if (S->co[id] == NULL)
	{
		return COROUTINE_DEAD;
	}
	return S->co[id]->status;
}

int coroutine_running(struct schedule *S)
{
	return S->running;
}
/*class SchedulerImpl
{
  public:
    explicit SchedulerImpl(int stacksize) : index_(0), running_(-1), stacksize_(stacksize) {}
    ~SchedulerImpl()
    {
        std::map<int, coroutine *>::iterator it = id2routine_.begin();

        while (it != id2routine_.end())
        {
            if (it->second)
                free(it->second);
        }
    }

    int CreateCoroutine(CoFunc func, void *arg)
    {
        coroutine *cor = (coroutine *)malloc(sizeof(coroutine) + stacksize_);

        if (cor == NULL)
            return -1;

        cor->arg = arg;

        // printf("arg == %d\n", *(int *)arg);

        cor->func = func;
        cor->status = CO_READY;

        int index = index_++;
        id2routine_[index] = cor;

        return index;
    }
    int DestroyCoroutine(int id)
    {
        coroutine *cor = id2routine_[id];
        if (!cor)
            return -1;

        free(cor);
        id2routine_.erase(id);
        return id;
    }

    int Yield()
    {
        if (running_ < 0)
            return -1;

        int cur = running_;
        running_ = -1;

        coroutine *cor = id2routine_[cur];

        cor->status = CO_SUSPENDED;

        swapcontext(&cor->cxt, &mainContext_);
        return 0;
    }
    int ResumeCoroutine(int id, int y)
    {
        coroutine *cor = id2routine_[id];
        if (cor == NULL || cor->status == CO_RUNNING)
            return 0;

        if ((-1 == running_) && (CO_FINISHED == cor->status))
            DestroyCoroutine(id);

        switch (cor->status)
        {
        case CO_READY:
        {
            getcontext(&cor->cxt);

            cor->status = CO_RUNNING;
            // cor->cxt.uc_stack.ss_sp = cor->stack;
            cor->cxt.uc_stack.ss_sp = cor->stack;
            cor->cxt.uc_stack.ss_size = stacksize_;
            // sucessor context.
            cor->cxt.uc_link = &mainContext_;

            running_ = id;
            // setup coroutine context
            makecontext(&cor->cxt, (void (*)())Schedule, 1, this);
            swapcontext(&mainContext_, &cor->cxt);
        }
        break;
        case CO_SUSPENDED:
        {
            running_ = id;
            cor->status = CO_RUNNING;
            swapcontext(&mainContext_, &cor->cxt);
        }
        break;
        }

        return y;
    }

    bool IsCoroutineAlive(int id) const
    {
        std::map<int, coroutine *>::const_iterator it = id2routine_.find(id);
        if (it == id2routine_.end())
            return false;

        return it->second;
    }

  private:
    SchedulerImpl(const SchedulerImpl &);
    SchedulerImpl &operator=(const SchedulerImpl &);

    static void Schedule(void *arg)
    {
        assert(arg);
        SchedulerImpl *sched = (SchedulerImpl *)arg;

        int running = sched->running_;

        coroutine *cor = sched->id2routine_[running];
        assert(cor);

        sched->running_ = -1;
        cor->status = CO_FINISHED;
    }

  private:
    int index_;
    int running_;
    const int stacksize_;

    ucontext_t mainContext_;
    std::map<int, coroutine *> id2routine_;
    coroutine **routine_;
};*/