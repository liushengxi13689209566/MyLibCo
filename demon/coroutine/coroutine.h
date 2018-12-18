#ifndef H_COROUTINE_H_
#define H_COROUTINE_H_

#include <stdint.h>
#include <map>
#include <stdlib.h>
#include <assert.h>
#include <ucontext.h>

typedef void (*CoFunc)(void *arg);

enum Status
{
    CO_READY,
    CO_SUSPENDED,
    CO_RUNNING,
    CO_FINISHED
};

struct coroutine
{
    void *arg;

    int status;
    ucontext_t cxt;
    CoFunc func;

    char stack[1024];
};

class SchedulerImpl
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
};

class CoroutineScheduler // non copyable
{
  public:
  public:
    explicit CoroutineScheduler(int stacksize = 1024) : impl_(new SchedulerImpl(stacksize)) {}
    ~CoroutineScheduler()
    {
        delete impl_;
    }

    int DestroyCoroutine(int id)
    {
        return impl_->DestroyCoroutine(id);
    }
    int CreateCoroutine(CoFunc func, void *arg)
    {
        return impl_->CreateCoroutine(func, arg);
    }

    int Yield(int y = 0)
    {
        return impl_->Yield();
    }
    int ResumeCoroutine(int id, int y = 0)
    {
        return impl_->ResumeCoroutine(id, y);
    }

    bool IsCoroutineAlive(int id) const
    {
        return impl_->IsCoroutineAlive(id);
    }

  private:
    CoroutineScheduler(const CoroutineScheduler &);
    CoroutineScheduler &operator=(const CoroutineScheduler &);

  private:
    class SchedulerImpl *impl_;
};

#endif
