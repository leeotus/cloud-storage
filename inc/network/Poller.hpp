#ifndef __CLOUD_STORAGE_POLLER_HPP__
#define __CLOUD_STORAGE_POLLER_HPP__

#include "network/Channel.hpp"
#include <map>
#include <vector>

namespace flkeeper {
namespace network {

class Poller {
public:
  using ChannelList = std::vector<Channel *>;

  Poller(EventLoop *loop);
  virtual ~Poller();

  /// @brief 轮询IO事件
  /// @param timeoutMs 超时时间
  /// @param activeChannels 活动的Channel列表
  /// @return 轮询时间
  virtual date::TimeStamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

  /// @brief 更新Channel
  virtual void updateChannel(Channel *channel) = 0;

  /// @brief 移除Channel
  virtual void removeChannel(Channel *channel) = 0;

  /// @brief 是否有Channel
  virtual bool hasChannel(Channel *channel) const;

  /// @brief 获取默认的Poller
  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const;

protected:
  using ChannelMap = std::map<int, Channel *>;
  ChannelMap channels_; // fd到Channel的映射

private:
  EventLoop *ownerLoop_;
};

} // namespace network
} // namespace flkeeper

#endif
