/*************************************************************************
	> File Name: CoroutineScheduler.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时00分36秒
 ************************************************************************/

#ifndef _COROUTINESCHEDULER_H
#define _COROUTINESCHEDULER_H

#include <ucontext.h>
#include <unordered_map>
#include <memory>
#include <functional>
// #include "./Coroutine.h"

#define INFO(x) std::cout << x << std::endl;

namespace Tattoo
{
using CoFun = std::function<void()>;
class Coroutine;

class CoroutineSchedule
{
  public:
	static const int STACK_SIZE = 1024 * 1024;
	static const int MAX_CO = 16;

	CoroutineSchedule() : cur_co_num_(0), cur_run_id_(-1) {}
	~CoroutineSchedule() {}

	CoroutineSchedule(const CoroutineSchedule &) = delete;
	CoroutineSchedule &operator=(const CoroutineSchedule &) = delete;

	int CreateCoroutine(CoFun func)
	{
		cur_co_num_++;
		auto cor = std::make_shared<Coroutine>(this, func, cur_co_num_);
		mmap_[cur_co_num_] = cor;
		return cur_co_num_;
	}
	void DestroyCroutine(int cor_id)
	{
		if (mmap_.find(cor_id) == mmap_.end())
			return;
		if (cor_id == cur_run_id_)
			cur_run_id_ = -1;
		mmap_[cor_id]->SetStatus(Coroutine::CO_DEAD);
		cur_co_num_--;
	}
	void ResumeCoroutine(int cor_id);
	void Yield();

	bool IsAlive(int cor_id);
	int GetCurCoID() { return cur_run_id_; }

  private:
	std::unordered_map<int, std::shared_ptr<Coroutine>> mmap_;
	char stack_[STACK_SIZE] = {0};
	ucontext_t main_ctx;
	int cur_co_num_; /*实时记录协程数量,也会控制map下标*/
	int cur_run_id_;
};
} // namespace Tattoo
#endif
