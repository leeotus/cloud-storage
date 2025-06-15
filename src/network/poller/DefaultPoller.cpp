#include "network/Poller.hpp"
#include "network/poller/EPollPoller.hpp"
#include "network/poller/PollPoller.hpp"

#include <stdlib.h>

namespace flkeeper {
namespace network {

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("MYMUDUO_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop); // 默认使用epoll
    }
}

}  // namespace network
}  // namespace flkeeper
