#ifndef H_COROUTINE_H_
#define H_COROUTINE_H_

#include <stdint.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ucontext.h>

typedef void (*CoFunc)(void *arg);
const int STACK_SIZE = (1024 * 1024);
const int DEFAULT_COROUTINE = 16;

enum Status
{
    CO_READY,
    CO_SUSPENDED,
    CO_RUNNING,
    CO_FINISHED
};

class Coroutine
{
  public:
    void *arg;
    CoFunc func;

    int status;
    ucontext_t cxt;
    char *stack;
};

class CoroutineScheduler
{
  public:
  public:
    explicit CoroutineScheduler(void) : running_(-1), index_(0) {}
    ~CoroutineScheduler() {}

    int DestroyCoroutine(int id)
    {
        return impl_->DestroyCoroutine(id);
    }
    int CreateCoroutine(CoFunc func, void *arg) //完成
    {

        class Coroutine *co = new Coroutine();
        co->func = func;
        co->arg = arg;
        co->status = CO_READY;
        co->stack = NULL;

        int index = index_++;
        mmap_[index] = co;
        return index;
    }

    int Yield()
    {
        return impl_->Yield();
    }
    void ResumeCoroutine(int id)
    {
        assert(-1 == running_); //当前没有运行的协程
        assert(id >= 0);

        Coroutine *cor = mmap_[id];
        if (NULL == cor || CO_RUNNING == cor->status)
            return;
        switch (cor->status)
        {
        case CO_READY:
        {
            getcontext(&cor->cxt);

            cor->status = CO_RUNNING;
            // cor->cxt.uc_stack.ss_sp = cor->stack;
            cor->cxt.uc_stack.ss_sp = cor->stack;
            cor->cxt.uc_stack.ss_size = STACK_SIZE;
            // sucessor context.
            cor->cxt.uc_link = &ctx_main_;

            running_ = id;
            // setup coroutine context
            makecontext(&cor->cxt, (void (*)())Schedule, 1, this);
            swapcontext(&ctx_main_, &cor->cxt);
        }
        break;
        case CO_SUSPENDED: /*待填充*/
        {
            // running_ = id;
            // cor->status = CO_RUNNING;
            // swapcontext(&mainContext_, &cor->cxt);
        }
        break;
        }
    }

    bool IsAlive(int id) const //完成
    {
        std::map<int, Coroutine *>::const_iterator it = mmap_.find(id);
        if (it == mmap_.end())
            return false;
        else
            return true;
    }

  private:
    CoroutineScheduler(const CoroutineScheduler &);
    CoroutineScheduler &operator=(const CoroutineScheduler &);

  private:
    char stack[STACK_SIZE];
    ucontext_t ctx_main_;
    int running_; //正在运行的　ＩＤ
    std::map<int, Coroutine *> mmap_;
    int index_;
};

#endif
