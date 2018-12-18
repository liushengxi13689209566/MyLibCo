/*************************************************************************
	> File Name: test.cpp
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年12月17日 星期一 22时33分08秒
 ************************************************************************/

#include <functional>
#include <iostream>

typedef std::function<void(void *)> Fun;

void *tt;
void print_num(void *arg)
{
}

class TT
{
  public:
	void Create(Fun func)
	{
	}
};
int main()
{
	// 存储到 std::bind 调用的结果
	TT test;
	test.Create(std::bind(print_num, 31337));
}