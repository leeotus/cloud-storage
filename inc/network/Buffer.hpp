#ifndef __CLOUD_STORAGE_BUFFER_HPP__
#define __CLOUD_STORAGE_BUFFER_HPP__

#include <cassert>
#include <cstring>
#include <vector>
#include <string>

#define BUFFER_LEN 65536

namespace flkeeper {
namespace network {

/**
 * @brief 网络库底层缓冲区类
 * Buffer类提供一个连续的内存缓冲区，用于网络读写操作. 缓冲区分为3个区域
 * +-----------------+---------------------+--------------------+
 * |   prependable   |       readable      |       writeable    |
 * ------------------+---------------------+--------------------+
 * | <--  已占用  --> | <-- 已读取/待处理 --> | <-- 可写入新数据 --> |
 * ^                 ^                     ^                    ^
 * 0              readIndex_            writeIndex_         buffer_.size()
 * prependable: 用于在数据前添加额外信息(如信息长度),避免数据移动
 * readable: 存储可读数据
 * writeable: 用于写入新数据
 */
class Buffer {
static const size_t kCheapPrepend = 8U;
static const size_t kInitialSize = 1024U;
public:
  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + kInitialSize), readIndex_(kCheapPrepend),
        writeIndex_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  const char *findCRLF() const;
  const char *findCRLF(const char *start) const;

  const char *findEOL() const;
  const char *findEOL(const char* start) const;

  /**
   * @brief 可读字节数
   */
  size_t readableBytes() const;

  /**
   * @brief 可写字节数
   */
  size_t writableBytes() const;

  /**
   * @brief 可前置字节数
   */
  size_t prependableBytes() const;

  /**
   * @brief 检测缓冲区是否有足够的可写空间
   * @param len 要求缓冲区可写空间的至少长度
   */
  void checkWritableSpace(size_t len);

   /// @brief 读取len个字节
    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    /// @brief 读取直到end位置
    void retrieveUntil(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    /// @brief 读取所有数据
    void retrieveAll() {
        readIndex_ = kCheapPrepend;
        writeIndex_ = kCheapPrepend;
    }

    /// @brief 读取所有数据并转换为string
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    /// @brief 读取len个字节并转换为string
    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

  // swap交换函数
  void swap(Buffer &rhs);

  /**
   * @brief 返回可读数据的起始位置
   */
  const char *peek() const;

  /**
   * @brief 追加数据
   * @param data 要追加到buffer_后的数据
   * @param len 追加数据的长度
   * @note char*类型的字符串使用额外指定字符串长度的参数len
   */
  void append(const char *data, size_t len);
  void append(const std::string& data);

  // @brief 追加前置数据
  void prepend(const void* data, size_t len);

  // @brief 收缩空间
  void shrink(size_t reserve);

  /**
   * @brief 返回缓冲区中的可写位置
   */
  char* beginWrite();
  const char* beginWrite() const;

  // 写入len个字节
  inline void hasWritten(size_t len) {
    assert(writableBytes() >= len);
    writeIndex_ += len;
  }
  // 回退写位置
  inline void unWrite(size_t len) {
    assert(len <= readableBytes());
    writeIndex_ -= len;
  }

  // @brief 缓冲区容量
  size_t capacity() const { return buffer_.capacity(); }

  /**
   * @brief 从fd文件描述符中读取数据
   * @param fd 文件描述符
   * @param savedErrono 保存错误码
   * @return ssize_t 读取数据的长度
   */
  ssize_t readFd(int fd, int *savedErrno);

private:
  /**
   * @brief 返回第一个字符
   * @note buffer_.begin()得到迭代器，对其*操作可以得到第一个字符
   * 最后返回第一个字符的地址(char *)
   */
  char *begin() { return &(*buffer_.begin()); }
  const char *begin() const { return &*buffer_.begin(); }

  void makeSpace(size_t len);

  std::vector<char> buffer_;    // 缓冲区
  size_t readIndex_;            // 读位置
  size_t writeIndex_;           // 写位置

  static const char kCRLF[];    // CRLF字符
};

}   // namespace network
}   // namespace flkeeper

#endif
