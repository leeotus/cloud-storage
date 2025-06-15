#ifndef __CLOUD_STORAGE_POLLPOLLER_HPP__
#define __CLOUD_STORAGE_POLLPOLLER_HPP__

#include <vector>
#include <map>

#include "network/EventLoop.hpp"
#include "network/Poller.hpp"
#include "utils/date/TimeStamp.hpp"

struct pollfd;

namespace flkeeper {
namespace network {


class PollPoller : public Poller {
public:
  PollPoller(EventLoop *loop);
  ~PollPoller() override = default;

  date::TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

  using PollFdList = std::vector<struct pollfd>;
  PollFdList pollfds_;
};

} // namespace network
}   // namespace flkeeper
#endif
