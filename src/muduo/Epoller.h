#include "EventLoop.h"
#include "Channel.h"

#include <map>

///
/// IO Multiplexing with epoll(4).
///
class EPollPoller
{
public:
  typedef std::vector<Channel *> ChannelList;

  EPollPoller(EventLoop *loop);
  ~EPollPoller() override;

  Timestamp poll(int timeoutMs, ChannelList *activeChannels);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

private:
protected:
  typedef std::map<int, Channel *> ChannelMap;
  ChannelMap channels_;

  EventLoop *ownerLoop_;
  static const int kInitEventListSize = 16;

  static const char *operationToString(int op);

  void fillActiveChannels(int numEvents,
                          ChannelList *activeChannels) const;
  void update(int operation, Channel *channel);

  typedef std::vector<struct epoll_event> EventList;

  int epollfd_;
  EventList events_;
};