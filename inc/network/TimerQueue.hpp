#ifndef __CLOUD_STORAGE_TIMERQUEUE_HPP__
#define __CLOUD_STORAGE_TIMERQUEUE_HPP__

#include <set>
#include <vector>

#include "utils/NonCopyable.hpp"
#include "utils/thread/PMutex.hpp"
#include "utils/date/TimeStamp.hpp"
#include "utils/Types.hpp"
#include "Callback.hpp"
#include "Channel.hpp"

namespace flkeeper {
namespace network {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : NonCopyable {
public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, date::TimeStamp when, DFLK_FP interval);

  void cancel(TimerId timerId);
private:
  using Entry = std::pair<date::TimeStamp, Timer*>;

  // 定时器集合，按到期时间排序
  using TimerList = std::set<Entry>;

  // 激活的定时器集合
  using ActiveTimer = std::pair<Timer*, DFLK_INT64>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);

  void handleRead();

  std::vector<Entry> getExpired(date::TimeStamp now);
  void reset(const std::vector<Entry>& expired, date::TimeStamp now);

  bool insert(Timer* timer);

  EventLoop *loop_;     // 该定时器队列所属的事件循环器
  const int timerfd_;     // 定时器文件描述符
  Channel timerfdChannel_;    // 定时器通道
  TimerList timers_;      // 按到期时间排序的定时器集合

  ActiveTimerSet activeTimers_;   // 按对象地址排序的定时器集合
  bool callingExpiredTimers_;     // 是否正在处理到期定时器
  ActiveTimerSet cancelingTimers_;    // 保存被取消的定时器
};

}     // namespace network
}     // namespace flkeeper

#endif
