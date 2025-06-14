#ifndef __CLOUD_STORAGE_FKEXCEPTION_HPP__
#define __CLOUD_STORAGE_FKEXCEPTION_HPP__

#include <exception>
#include <string>
#include <vector>

namespace flkeeper {

class FKException : public std::exception {
public:
  explicit FKException(char *what);
  explicit FKException(const char *what);

  ~FKException() noexcept override = default;

  /**
   * @brief 获取异常信息
   * @return const char* 返回异常信息的字符串
   */
  const char* what() const noexcept override {
    return message_.c_str();
  }

  /**
   * @brief 获取堆栈跟踪信息
   * @return const char* 堆栈跟踪字符串
   */
  const char *stackTraceInfo() const noexcept {
    return stack_.c_str();
  }

private:

  /**
   * @brief 生成当前堆栈的跟踪信息
   */
  void trace();

  std::string message_;     // 异常信息
  std::string stack_;       // 堆栈跟踪信息
};

} // namespace flkeeper

#endif
