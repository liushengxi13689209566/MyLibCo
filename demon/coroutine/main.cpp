/*************************************************************************
	> File Name: main.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月19日 星期三 12时01分56秒
 ************************************************************************/

#include "Coroutine.h"

#include <iostream>
#include <unistd.h>
using namespace std;
using namespace Tattoo;

void func1(std::shared_ptr<CoroutineSchedule> s, void *arg)
{
	cout << "1" << endl;
	cout << "2" << endl;
	s->Yield();
	cout << "3" << endl;
	s->Yield();
}
void func2(std::shared_ptr<CoroutineSchedule> s, void *arg)
{
	cout << "a" << endl;
	cout << *(int *)arg << endl;

	s->Yield();
	cout << "b" << endl;
	cout << "c" << endl;
	s->Yield();
}
class TEST
{
  public:
	void func3(std::shared_ptr<CoroutineSchedule> s, void *arg)
	{
		cout << "x" << endl;
		cout << *(int *)arg << endl;
		s->Yield();
		cout << "y" << endl;
		cout << "z" << endl;
		s->Yield();
	}
};

int main()
{
	std::shared_ptr<CoroutineSchedule> schedule = std::make_shared<CoroutineSchedule>();

	int test1 = 6666;
	int test2 = 9999;

	int id_co1 = schedule->CreateCoroutine(std::bind(&func1, schedule, &test1));
	int id_co2 = schedule->CreateCoroutine(std::bind(&func2, schedule, &test2));

	printf("main start\n");
	while (1)
	{
		if (schedule->IsAlive(id_co1))
			schedule->ResumeCoroutine(id_co1);

		if (schedule->IsAlive(id_co2))
			schedule->ResumeCoroutine(id_co2);
		sleep(1);
	}
	printf("main end\n");

	return 0;
}
