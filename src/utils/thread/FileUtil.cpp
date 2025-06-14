#include "utils/FileUtil.hpp"

#include <cassert>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

namespace flkeeper {

bool readFile(std::string filename, std::string *content, size_t sizeLimt) {
  ReadSmallFile file(filename);
  return file.readToString(sizeLimt, content, nullptr, nullptr, nullptr);
}

/**============================================
 *!               ReadSmallFile类函数
 *=============================================**/
ReadSmallFile::ReadSmallFile(StringArg filename) : fd_(-1), err_(0) {
  fd_ = open(filename.c_str(), O_RDONLY | O_CLOEXEC);
  if(fd_ < 0) {
    err_ = errno;
  }
  memset(&buf_, 0, READ_BUFFER_LEN);
}

ReadSmallFile::~ReadSmallFile() {
  if(fd_ >= 0) {
    ::close(fd_);
  }
}

int ReadSmallFile::readToString(size_t maxSize, StringArg *content,
                                DFLK_INT64 *fileSize, DFLK_INT64 *modifyTime,
                                DFLK_INT64 *createTime) {

  static_assert(sizeof(off_t) == 8, "__FILE_OFFSET__BITS = 64");
  assert(content != nullptr);
  int err = err_;
  if(fd_ >= 0) {
    struct stat info;     // 保存文件信息
    if(!::fstat(fd_, &info)) {
      if(S_ISREG(info.st_mode)) {
        // 如果是普通文件
        *fileSize = info.st_size;   // 获取文件的大小
        content->reserve(std::min(maxSize, static_cast<size_t>(*fileSize)));
      } else if(S_ISDIR(info.st_mode)) {
        err = EISDIR;
      }

      if(modifyTime) {
        *modifyTime = info.st_mtim.tv_sec;
      }
      if(createTime) {
        *createTime = info.st_ctim.tv_sec;
      } else {
        err = errno;    // 获取文件信息失败
      }
    }

    while(content->size() < maxSize) {
      size_t toRead = std::min(maxSize-content->size(), sizeof(buf_));
      ssize_t n = read(fd_, buf_, toRead);
      if(n > 0) {
        content->append(buf_, n);
      } else {
        if(n < 0) {
          err = errno;
        }
        break;
      }
    }
  }
  return err;
}

int ReadSmallFile::readToBuffer(char *buffer, int size) {
  int err = err_;
  if(fd_ >= 0) {
    // @note: pread类似于read函数，但是增加了offset偏移量,可以在不改变文件当前偏移量的情况下
    // 从文件中的任意位置读取数据
    ssize_t n = ::pread(fd_, buffer, size, 0);
    if(n >= 0) {
      if(n < size) {
        buffer[n] = '\0';
      }
      return 0;
    }
    err = errno;
  }
  return errno;
}

/**============================================
 *!               AppendFile类函数
 *=============================================**/

// @brief 无锁方式的写操作
size_t AppendFile::write(const char *logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

AppendFile::AppendFile(StringArg filename) : fp_(nullptr), writtenBytes_(0) {
  fp_ = ::fopen(filename.c_str(), "ae");
  assert(fp_ != nullptr);

  // 为文件流设置一个缓冲区
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

void AppendFile::append(const char* logline, size_t len) {
  size_t written = 0;
  while(written != len) {
    auto remain = len - written;
    auto n = write(logline+written, remain);
    if(n != remain) {
      int err = ferror(fp_);
      if(err) {
        fprintf(stderr, "AppendFile::append() failed %s\r\n", strerror(err));
        break;
      }
    }
    written += n;
  }
  writtenBytes_ += written;
}

void AppendFile::flush() {
  ::fflush(fp_);
}

} // namespace flkeeper
