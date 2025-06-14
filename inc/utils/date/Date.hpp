#ifndef __CLOUD_STORAGE_DATE_HPP__
#define __CLOUD_STORAGE_DATE_HPP__

#include "utils/Types.hpp"
#include "utils/Copyable.hpp"

struct tm;

namespace flkeeper {

namespace date {

class Date: public Copyable {
public:

  // 年月日结构体
  struct YearMonthDay {
    DFLK_INT8 day;
    DFLK_INT8 month;
    DFLK_INT16 year;
  };

  Date() : julianDayNumber_(0) {}

  Date(DFLK_INT16 year, DFLK_INT8 month, DFLK_INT8 day);

  explicit Date(DFLK_UINT32 julianDayNumber) : julianDayNumber_(julianDayNumber) {}

  explicit Date(const struct tm& t);

  /**
   * @brief 转换为YearMonthDay结构
   */
  YearMonthDay yearMonthDay() const;

  /**
   * @brief 获取Date类对象中存储的儒略日数
   */
  DFLK_INT64 julianDayNumber() const;

  // 获取年,月,日:
  DFLK_INT16 year() const { return yearMonthDay().year; }
  DFLK_INT8 month() const { return yearMonthDay().month; }
  DFLK_INT8 day() const { return yearMonthDay().day; }

  /**
   * @brief 返回一周中的第几天
   * @note 0表示周日，6表示周六
   */
  DFLK_INT8 weekOfDay() const;

  /**
   * @brief 转换为字符串表示
   */
  std::string toString() const;

private:
  DFLK_UINT64 julianDayNumber_;   // 儒略日数,用于内部计算
};

inline bool operator<(const Date &lhs, const Date &rhs) {
  return lhs.julianDayNumber() < rhs.julianDayNumber();
}

inline bool operator==(const Date &lhs, const Date &rhs) {
  return lhs.julianDayNumber() == rhs.julianDayNumber();
}

} // namespace date

}   // namesapce flkeeper

#endif
