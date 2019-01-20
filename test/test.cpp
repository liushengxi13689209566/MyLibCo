/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 18时13分56秒
 ************************************************************************/

#include <iostream>
using namespace std;

class noncopyable
{
  public:
	noncopyable(const noncopyable &) = delete;
	void operator=(const noncopyable &) = delete;

  protected:
	noncopyable() = default;
	~noncopyable() = default;
};
class Test : noncopyable
{
  public:
	Test()
	{
	}
};
int main(void)
{
	Test test;
	Test test4 = test;
}
