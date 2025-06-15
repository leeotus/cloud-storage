#ifndef __CLOUD_STORAGE_TIMER_HPP__
#define __CLOUD_STORAGE_TIMER_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/date/TimeStamp.hpp"
#include "utils/Types.hpp"
#include <functional>
#include <atomic>

namespace flkeeper {

namespace network {

class Timer : NonCopyable {
public:
  using TimerCallback = std::function<void()>;

  Timer(TimerCallback cb, date::TimeStamp when, DFLK_FP interval)
      : callback_(std::move(cb)), expiration_(when), interval_(interval),
        repeat_(interval > 0.0), sequence_(s_numCreated_.fetch_add(1)) {}

  // @brief 执行回调函数
  void run() { callback_(); }

  // @brief 返回定时器超时时间
  date::TimeStamp expiration() const { return expiration_; }

  // @brief 该定时器是否是重复的
  bool repeat() const { return repeat_; }

  // @brief 记录有多少个定时器已经被创建
  static DFLK_INT64 s_numCreated() { return s_numCreated_.load(); }

  // @brief 返回定时器的序列号
  DFLK_INT64 sequence() const { return sequence_; }

  void restart(date::TimeStamp now);

private:
  const TimerCallback callback_;    // 定时器回调函数
  date::TimeStamp expiration_;      // 定时器超时时间, 超时后触发回调函数
  const double interval_;   // 超时时间间隔
  const bool repeat_;       // 是否重复
  const DFLK_INT64 sequence_;   // 定时器的序号

  static std::atomic<DFLK_INT64> s_numCreated_;   // 定时器计数。用于生成sequence
};

}   // namespace network

} // namespace flkeeper

#endif
