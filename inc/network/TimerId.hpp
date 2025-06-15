#ifndef __CLOUD_STORAGE_TIMERID_HPP__
#define __CLOUD_STORAGE_TIMERID_HPP__

#include "utils/Copyable.hpp"
#include "utils/Types.hpp"

namespace flkeeper {
namespace network {

class Timer;

class TimerId : public Copyable {
public:
  TimerId() : timer_(nullptr), sequence_(0) {}

  TimerId(Timer *timer, DFLK_INT64 sequence)
      : timer_(timer), sequence_(sequence) {}

  friend class TimerQueue;
private:
  Timer* timer_;    // 定时器指针
  DFLK_INT64 sequence_;     // 定时器序号
};

} // namespace network
} // namespace flkeeper

#endif
