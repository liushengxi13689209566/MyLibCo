/*************************************************************************
	> File Name: Epoll.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 22时07分58秒
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H
#include <vector>
#include <ctime>
#include <map>
struct epoll_event;
namespace Tattoo
{
class EventLoop;
class Channel;

// 类似于　　stCoEpoll_t *pEpoll;　结构　
//这里的网络模型架构我还是想采取　
// 一个线程一个事件循环 一个线程里面再有指定数量的协程去处理所到的事件
class Epoll
{
	typedef std::vector<Channel *> ChannelList;

  public:
	Epoll(EventLoop *loop);
	~Epoll();

	time_t pool(int timeoutMs, ChannelList *activeChannels);
	void updateChannel(Channel *channel);
	void removeChannel(Channel *channel);

	void fillActiveChannels(int numEvents,
							ChannelList *activeChannels) const;
	void update(int operation, Channel *channel);
	//debug
	const char *operationToString(int op);

  private:
	static const int kInitEventListSize = 16;
	typedef std::map<int, Channel *> ChannelMap;
	ChannelMap channels_;

	typedef std::vector<struct epoll_event> EventList;
	int epollfd_;
	EventList events_;

	EventLoop *ownerLoop_;
};
} // namespace Tattoo
#endif
