#include "network/Timer.hpp"
#include <atomic>

namespace flkeeper {
namespace network {

std::atomic<DFLK_INT64> Timer::s_numCreated_;

void Timer::restart(date::TimeStamp now) {
  if(repeat_) {
    expiration_ = date::addTime(now, interval_);
  } else {
    expiration_ = date::TimeStamp::invalid();
  }
}

}   // namespace network
}   // namespace flkeeper

