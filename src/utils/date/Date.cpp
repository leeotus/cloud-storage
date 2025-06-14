#include "utils/date/Date.hpp"

namespace flkeeper {

namespace date {

/**
 * @brief 将日期转换为儒略日数
 * @param year 年份
 * @param month 月份
 * @param day 日数
 * @return int 返回儒略日数
 * @note 算法来自:http://www.faqs.org/faqs/calendars/faq/part2/
 */
static DFLK_INT64 getJulianDayNumber(DFLK_INT16 year, DFLK_INT8 month, DFLK_INT8 day) {
  DFLK_INT64 a = (14 - month) / 12;
  DFLK_INT64 y = year + 4800 - a;
  DFLK_INT64 m = month + 12 * a - 3;
  return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
}

static Date::YearMonthDay getYearMonthDay(DFLK_INT64 julianDayNumber) {
  DFLK_INT64 a = julianDayNumber + 32044;
  DFLK_INT64 b = (4 * a + 3) / 146097;
  DFLK_INT64 c = a - ((146097 * b) / 4);
  DFLK_INT64 d = (4 * c + 3) / 1461;
  DFLK_INT64 e = c - ((1461 * d) / 4);
  DFLK_INT64 m = (5 * e + 2) / 153;

  Date::YearMonthDay ymd;
  ymd.day = e - ((153 * m + 2) / 5) + 1;
  ymd.month = m + 3 - 12 * (m / 10);
  ymd.year = 100 * b + d - 4800 + (m / 10);
  return ymd;
}

Date::Date(DFLK_INT16 year, DFLK_INT8 month, DFLK_INT8 day) {}

Date::Date(const struct tm &t)
    : julianDayNumber_(
          getJulianDayNumber(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday)) {}

DFLK_INT64 Date::julianDayNumber() const {
  return julianDayNumber_;
}

Date::YearMonthDay Date::yearMonthDay() const {
  return getYearMonthDay(julianDayNumber_);
}

DFLK_INT8 Date::weekOfDay() const {
  return (julianDayNumber_ + 4) % 7;
}

std::string Date::toString() const {
  char buf[32];
  YearMonthDay ymd = yearMonthDay();
  snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
  return buf;
}

} // namespace date

} // namespace flkeeper
