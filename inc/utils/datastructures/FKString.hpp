#ifndef __CLOUD_STORAGE_FLKSTRING_HPP__
#define __CLOUD_STORAGE_FLKSTRING_HPP__

#include "utils/Types.hpp"
#include <string>

namespace flkeeper {

/**
 * @brief 轻量级字符串视图类
 * 用于高效传递字符串,不拥有字符串数据
 * @note 清除FKString对象不会导致原本的字符串数据被清除
 */
class FKString {
public:
  FKString();
  FKString(const char *str);
  FKString(const unsigned char *str);
  FKString(const std::string &str);
  FKString(const char* offset, int len);

  /**
   * @brief 获取字符串
   */
  const char *data() const;

  /**
   * @brief 获取字符串的长度
   */
  int size() const;

  /**
   * @brief 判断是否是空字符串
   */
  bool empty() const;

  /**
   * @brief 清除字符串
   * @note sptr_指针只指向字符串而不分配内存保存字符串,
   * 所以清空字符串直接将sptr成员指向空指针
   */
  void clear();

  /**
   * @brief 设置字符串
   * @param str 指向字符串的指针
   * @param len 指定当前的字符串视图的长度
   * @note 字符串视图的长度不一定要等于输入字符串的长度
   */
  void set(const char *str, DFLK_INT64 len);
  void set(const char *str);
  void set(void* buffer, DFLK_INT64 len);

  char operator[](int index) const;

  // 去除前缀n个字符
  void remove_prefix(int n);

  // 去除后缀n个字符
  void remove_suffix(int n);

  /**
   * @brief 判断两个fkstring是否是相同的
   * @param fkstr 要比较的fkstring字符串
   * @return 相同返回true,失败返回false
   */
  bool operator==(const FKString& fkstr) const;
  bool operator!=(const FKString &fkstr) const;

  /**
   * @brief 将char字符串转换为std::string字符串
   * @return std::string 返回转换好的std::string字符串
   */
  std::string as_string() const;

  /**
   * @brief 将FKString字符串拷贝到输入的std::string字符串
   * @param target 输入的std::string字符串，需要将FKString里保存的字符串拷贝到其中
   */
  void CopyToString(std::string *target) const;

  /**
   * @brief 判断当前字符串是否以输入的FKString字符串为开头
   * @param fkstr 要判断是否以该FKString字符串为开头
   */
  bool StartWith(const FKString& fkstr) const;

private:
  const char *sptr_;
  DFLK_INT64 length_;
};

} // namespace flkeeper

#endif
