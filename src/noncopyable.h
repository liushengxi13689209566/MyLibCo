/*************************************************************************
	> File Name: noncopyable.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 18时20分12秒
 ************************************************************************/

#ifndef _NONCOPYABLE_H
#define _NONCOPYABLE_H
namespace Tattoo
{
class noncopyable
{
  public:
	noncopyable(const noncopyable &) = delete;
	void operator=(const noncopyable &) = delete;

  protected:
	noncopyable() = default;
	~noncopyable() = default;
};
} // namespace Tattoo
#endif
