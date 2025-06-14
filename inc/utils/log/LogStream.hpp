#ifndef __CLOUD_STORAGE_LOG_STREAM_HPP__
#define __CLOUD_STORAGE_LOG_STREAM_HPP__

#include "utils/datastructures/FKString.hpp"
#include "utils/NonCopyable.hpp"
#include <cstring>

namespace flkeeper {
namespace log {

constexpr DFLK_INT32 smallBufferLen = 4000;
constexpr DFLK_INT32 largeBufferLen = 4000 * 1000;

template <DFLK_INT32 SIZE>
class FixedBuffer : NonCopyable {
public:
  FixedBuffer() : cur_(data_) {}

  /**
   * @brief 往数据缓冲区末尾添加数据
   * @param buf 要添加的数据
   * @param len 要添加的数据的长度
   */
  void append(const char* buf, size_t len) {
    if(avail() > static_cast<DFLK_INT32>(len)) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  // @brief 获取缓冲区存储的数据
  const char* data() const {
    return data_;
  }

  // @brief 缓冲区保存数据的长度
  DFLK_INT32 length() const {
    return static_cast<DFLK_INT32>(cur_-data_);
  }

  // @brief 获取数据末尾
  char *current() { return cur_; }

  // @brief 获取缓冲区还剩下多少字节
  DFLK_INT32 avail() const { return static_cast<DFLK_INT32>(end() - cur_); }

  // @brief 清空数据缓冲区后需要调用该函数来修改缓冲区指针
  void reset() { cur_ = data_; }

  // @brief 清空数据缓冲区的所有数据
  void bzero() { memset(data_, 0, sizeof(data_)); }

  // @brief 往缓冲区加入数据之后需要使用该函数修改cur_指针的位置
  void add(size_t len) { cur_ += len; }

  std::string toString() const { return std::string(data_, length()); }

private:
  char data_[SIZE];     // 缓冲区
  char *cur_;           // 指向缓冲区存储的数据的末尾

  const char *end() const { return data_+sizeof(data_); }
};

class LogStream : NonCopyable {
  static constexpr DFLK_INT32 kMaxNumericSize = 48;

public:
  typedef FixedBuffer<smallBufferLen> Buffer;
  LogStream &operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  LogStream &operator<<(short);
  LogStream &operator<<(unsigned short);
  LogStream &operator<<(int);
  LogStream &operator<<(unsigned int);
  LogStream &operator<<(long);
  LogStream &operator<<(unsigned long);
  LogStream &operator<<(long long);
  LogStream &operator<<(unsigned long long);

  LogStream &operator<<(const void *);
  LogStream &operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  LogStream &operator<<(double);

  LogStream &operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  LogStream &operator<<(const char *str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  LogStream &operator<<(const unsigned char *str) {
    return operator<<(reinterpret_cast<const char *>(str));
  }

  LogStream &operator<<(const std::string &v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  LogStream &operator<<(const FKString &v) {
    buffer_.append(v.data(), v.size());
    return *this;
  }

  LogStream &operator<<(const Buffer &v) {
    *this << v.toString();
    return *this;
  }

  // @brief 将数据放入缓冲区中
  void append(const char *data, size_t len);

  // @brief 获取数据缓冲区
  const Buffer& buffer() const { return buffer_; }

  // @brief 重置数据缓冲区
  void resetBuffer() { buffer_.reset(); }
private:
  template<typename T>
  void formatInteger(T v);

  Buffer buffer_;   // 缓冲区
};

class Fmt {

};

} // namespace log
} // namespace flkeeper

#endif
