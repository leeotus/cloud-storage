#include "network/poller/EPollPoller.hpp"
#include "network/Channel.hpp"
#include "utils/log/Logging.hpp"
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace flkeeper;
using namespace flkeeper::network;

// 在Linux上，EPOLL_CLOEXEC = O_CLOEXEC
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew = -1;    // channel未添加到poller中
const int kAdded = 1;   // channel已添加到poller中
const int kDeleted = 2; // channel已从poller中删除
} // namespace

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  LOG_TRACE << "fd total count " << channels_.size();

  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);
  int savedErrno = errno;
  TimeStamp now(TimeStamp::now());

  if (numEvents > 0) {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    // 如果events_的大小不足，扩容
    if (implicit_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG_TRACE << "nothing happened";
  } else {
    // 出错情况
    if (savedErrno != EINTR) {
      errno = savedErrno;
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return now;
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList *activeChannels) const {
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
    int fd = channel->fd();
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
            << " index = " << index;

  if (index == kNew || index == kDeleted) {
    // 新的channel或已删除的channel
    int fd = channel->fd();
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else { // index == kDeleted
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    // 更新已存在的channel
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel *channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
            << " fd = " << fd << " event = { " << channel->eventsToString()
            << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation)
                 << " fd =" << fd;
    } else {
      LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation)
                   << " fd =" << fd;
    }
  }
}

const char *EPollPoller::operationToString(int op) {
  switch (op) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    assert(false && "ERROR op");
    return "Unknown Operation";
  }
}
