#ifndef __CLOUD_STORAGE_FILEUTIL_HPP__
#define __CLOUD_STORAGE_FILEUTIL_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/Types.hpp"

namespace flkeeper {

template<typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

/**
 * @brief 读取文件的内容
 * @param filename 要读取的文件
 * @param content 输出的参数，存储文件内容
 * @param sizeLimt 读取大小限制，默认为1MB
 * @return 成功返回true，失败返回false
 */
bool readFile(std::string filename, std::string *content,
              size_t sizeLimt = 1024 * 1024);

/**
 * @brief 文件读取类
 * @note 以RAII的方式来管理文件描述符
 */
class ReadSmallFile : NonCopyable {
  using StringArg = std::string;
public:
  static constexpr DFLK_INT32 READ_BUFFER_LEN = 64 * 1024;

  ReadSmallFile(StringArg filename);
  ~ReadSmallFile();

  /**
   * @brief 读取文件内容到缓冲区buffer
   * @param maxSize 读取大小的限制
   * @param content 输出参数，存储文件内容
   * @param fileSize
   * @param modifyTime 记录文件的修改时间
   * @param createTime 记录文件的创建时间
   * @return int
   */
  int readToString(size_t maxSize, StringArg *content,
                   DFLK_INT64 *fileSize = nullptr,
                   DFLK_INT64 *modifyTime = nullptr,
                   DFLK_INT64 *createTime = nullptr);

  /**
   * @brief 读取文件内容到buffer
   * @param buf 输出缓冲区
   * @param size 缓冲区的大小
   * @return int 成功返回0,失败返回Errno
   */
  int readToBuffer(char *buf, int size);

private:
  int fd_;      // 文件描述符
  int err_;     // 错误码
  char buf_[READ_BUFFER_LEN];    // 64KB的缓冲区
};

/**
 * @brief 文件追加写入类
 */
class AppendFile : NonCopyable {
  using StringArg = std::string;
public:
  explicit AppendFile(StringArg filename);
  ~AppendFile();

  /**
   * @brief 追加写入数据
   * @param logline 要写入的数据
   * @param len 数据的长度
   */
  void append(const char* logline, size_t len);

  // @brief 刷新文件缓冲区到磁盘
  void flush();

  off_t writtenBytes() const { return writtenBytes_; }

private:
  // @brief 无锁方式写入数据
  size_t write(const char *logline, size_t len);

  FILE *fp_;      // 文件指针
  char buffer_[64 * 1024];    // 用户态缓冲区, 64kB
  off_t writtenBytes_;        // 已经写入的字节数
};

}  // namespace flkeeper

#endif
