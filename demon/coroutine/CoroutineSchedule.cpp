/*************************************************************************
	> File Name: CoroutineScheduler.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时00分36秒
 ************************************************************************/

#include "Coroutine.h"

namespace Tattoo
{

int CoroutineSchedule::CreateCoroutine(CoFun func)
{
	/*这里可以用 cur_max_num_　去限制协程数目*/
	cur_co_num_++;
	auto cor = std::make_shared<Coroutine>(this, func, cur_co_num_);
	mmap_[cur_co_num_] = cor;
	return cur_co_num_;
}
void CoroutineSchedule::DestroyCroutine(int cor_id)
{
	if (mmap_.find(cor_id) == mmap_.end())
		return;
	if (cor_id == cur_run_id_)
		cur_run_id_ = -1;
	mmap_[cor_id]->SetStatus(Coroutine::CO_FINSHED);
	cur_co_num_--; /*下次重复使用就行了，不需要 erase */
}
void CoroutineSchedule::Yield()
{
	if (-1 == cur_run_id_)
		return;
	int id = cur_run_id_;
	std::shared_ptr<Coroutine> cor = mmap_[id];
	// assert(reinterpret_cast<char *>(&cor) > );
	cor->save_stack(SchStack + STACK_SIZE);
	cor->SetStatus(Coroutine::CO_SUSPEND);
	cur_run_id_ = -1;
	swapcontext(&cor->ctx_, &main_ctx);
}
bool CoroutineSchedule::IsAlive(int cor_id)
{
	if (mmap_[cor_id]->GetStatus() == Coroutine::CO_FINSHED)
		return false;
	else
		return true;
}
void CoroutineSchedule::ResumeCoroutine(int cor_id)
{
	// INFO("*******************************************ResumeCoroutine", cor_id, cur_run_id_);

	if (mmap_.find(cor_id) == mmap_.end()) /*不存在的 id */
		return;

	assert(this->cur_run_id_ == -1); //保证没有其他协程运行

	std::shared_ptr<Coroutine> cor = mmap_[cor_id];
	switch (cor->GetStatus())
	{
	case Coroutine::CO_READY:
	{
		getcontext(&cor->ctx_);
		cor->ctx_.uc_stack.ss_sp = SchStack;
		cor->ctx_.uc_stack.ss_size = STACK_SIZE;
		cor->ctx_.uc_link = &main_ctx;

		cur_run_id_ = cor_id;
		cor->SetStatus(Coroutine::CO_RUNNING);
		makecontext(&cor->ctx_, reinterpret_cast<void (*)()>(static_fun), 1, this);
		swapcontext(&this->main_ctx, &cor->ctx_);
	}
	break;
	case Coroutine::CO_SUSPEND:
	{
		memcpy(SchStack + STACK_SIZE - cor->stack_cur_size_, cor->CorStack, cor->stack_cur_size_);
		cur_run_id_ = cor_id;
		cor->SetStatus(Coroutine::CO_RUNNING);
		swapcontext(&main_ctx, &cor->ctx_);
	}
	break;
	default:
		assert(0);
	}
	if (-1 == cur_run_id_ && cor->status_ == Coroutine::CO_FINSHED)
		DestroyCroutine(cor_id);
}
void CoroutineSchedule::static_fun(void *arg)
{
	CoroutineSchedule *sch = reinterpret_cast<CoroutineSchedule *>(arg);
	int id = sch->cur_run_id_;
	std::shared_ptr<Coroutine> cor = sch->mmap_[id];
	cor->func_();

	sch->cur_run_id_ = -1;
	sch->mmap_[id]->SetStatus(Coroutine::CO_FINSHED);
	delete[] cor->CorStack;
}
} // namespace Tattoo
