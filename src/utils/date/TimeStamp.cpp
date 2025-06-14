#include "utils/date/TimeStamp.hpp"

#include <inttypes.h>
#include <sys/time.h>
#include <stdio.h>

namespace flkeeper {
namespace date {
std::string TimeStamp::toString() const {
  char buf[32] = {0};
  DFLK_INT64 seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSec;
  DFLK_INT64 microSecs = microSecondsSinceEpoch_ % kMicroSecondsPerSec;
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microSecs);
  return buf;
}

std::string TimeStamp::toFormattedString(bool showMicroSec) const {
  char buf[64] = {0};
  struct tm tm_time;
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSec);
  gmtime_r(&seconds, &tm_time);

  if(showMicroSec) {
    int microseconds =
        static_cast<DFLK_INT64>(microSecondsSinceEpoch_ % kMicroSecondsPerSec);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

time_t TimeStamp::secondsSinceEpoch() const {
  return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSec);
}

TimeStamp TimeStamp::now() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  DFLK_INT64 seconds = tv.tv_sec;
  return TimeStamp(seconds & kMicroSecondsPerSec + tv.tv_usec);
}

void TimeStamp::add(DFLK_FP sec) {
  DFLK_INT64 delta = static_cast<DFLK_INT64>(sec * kMicroSecondsPerSec);
  microSecondsSinceEpoch_ += delta;
}

} // namespace date
} // namespace flkeeper
