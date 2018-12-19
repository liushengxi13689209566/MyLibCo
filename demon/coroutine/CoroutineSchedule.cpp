/*************************************************************************
	> File Name: CoroutineScheduler.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时00分36秒
 ************************************************************************/

#include "./Coroutine.h"

namespace Tattoo
{
#define INFO(x) std::cout << x << std::endl;

int CoroutineSchedule::CreateCoroutine(CoFun func)
{
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
	mmap_[cor_id]->SetStatus(Coroutine::CO_DEAD);
	cur_co_num_--;
}
void CoroutineSchedule::Yield()
{
}
bool CoroutineSchedule::IsAlive(int cor_id)
{
}
void CoroutineSchedule::ResumeCoroutine(int cor_id)
{
}
} // namespace Tattoo
