#ifndef __CLOUD_STORAGE_EPOLLPOLLER_HPP__
#define __CLOUD_STORAGE_EPOLLPOLLER_HPP__

#include "network/Poller.hpp"
#include <sys/epoll.h>
#include <vector>

namespace flkeeper {
namespace network {

/// @brief epoll(4)的封装
class EPollPoller : public Poller {
public:
  EPollPoller(EventLoop *loop);
  ~EPollPoller() override;

  /// @brief 重写基类Poller的抽象方法
  date::TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  static const int kInitEventListSize = 16; ///< 初始事件列表大小

  /// @brief 将活跃的channel填充到activeChannels中
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  /// @brief 更新channel在epoll中的状态
  /// @param operation 操作类型：EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
  void update(int operation, Channel *channel);

  /// @brief 将epoll操作转换为字符串，用于日志
  static const char *operationToString(int op);

  using EventList = std::vector<struct epoll_event>; ///< epoll事件列表类型

  int epollfd_;      ///< epoll文件描述符
  EventList events_; ///< 用于存放epoll返回的事件
};

} // namespace network
} // namespace flkeeper

#endif
