#ifndef __CLOUD_STORAGE_TIMESTAMP_HPP__
#define __CLOUD_STORAGE_TIMESTAMP_HPP__

#include "utils/Types.hpp"
#include "utils/Copyable.hpp"

namespace flkeeper {

namespace date {

/**
 * @brief 时间戳类，精确到微秒
 * @note 1s = 1000ms = 1000 * 1000 us
 */
class TimeStamp : public Copyable {
public:
  static const DFLK_INT64 kMicroSecondsPerSec = 1000 * 1000;
  TimeStamp() : microSecondsSinceEpoch_(0) {}

  explicit TimeStamp(DFLK_INT64 microSec) : microSecondsSinceEpoch_(0) {}

  // @brief 交换两个时间戳
  void swap(TimeStamp &that) {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  /**
   * @brief 将时间戳转换为字符串
   * @return std::string 格式化的时间字符串
   */
  std::string toString() const;

  /**
   * @brief 转换为格式化的字符串
   * @param showMicroSec 是否显示微妙,默认是显示
   * @return std::string 返回格式化的时间字符串
   */
  std::string toFormattedString(bool showMicroSec = true) const;

  inline DFLK_INT64 microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }


  // @brief 获取秒级时间戳
  time_t secondsSinceEpoch() const;

  // @brief 获取当前时间
  // @note 时间戳核心函数, 主要使用"gettimeofday"来获取
  static TimeStamp now();

  // @brief 获取一个无效的时间戳
  static TimeStamp invalid() { return TimeStamp(); }

  /**
   * @brief 为时间戳增加时间，以秒为输入参数
   * @param sec 需要为时间戳增加的时间秒数
   */
  void add(DFLK_FP sec);

  // @brief 从Unix时间戳创建TimeStamp
  static TimeStamp fromUnixTime(time_t t) {
    return fromUnixTime(t, 0);
  }

  // @brief 从Unix时间戳和微秒创建TimeStamp
  static TimeStamp fromUnixTime(time_t t, int microseconds) {
    return TimeStamp(static_cast<DFLK_INT64>(t) * kMicroSecondsPerSec + microseconds);
  }

private:
  DFLK_INT64 microSecondsSinceEpoch_;   // 保存的时间戳timestamp,从Unix纪元开始的微秒数
};

// @brief 比较两个时间戳
inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

/**
 * @brief 计算两个时间戳相差的秒数
 * @param ts1 被减时间戳
 * @param ts2 减数时间戳
 * @return DFLK_FP 返回相差的秒数
 */
inline DFLK_FP timeDiff(const TimeStamp &ts1, const TimeStamp &ts2) {
  DFLK_INT64 diff = ts1.microSecondsSinceEpoch() - ts2.microSecondsSinceEpoch();
  return static_cast<DFLK_FP>(diff) / TimeStamp::kMicroSecondsPerSec;
}

inline TimeStamp addTime(TimeStamp ts, DFLK_FP seconds) {
  DFLK_INT64 delta = static_cast<DFLK_INT64>(seconds * TimeStamp::kMicroSecondsPerSec);
  return TimeStamp(ts.microSecondsSinceEpoch() + delta);
}

} // namespace date

} // namespace flkeeper

#endif
