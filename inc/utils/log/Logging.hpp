#ifndef __CLOUD_STORAGE_LOGGING_HPP__
#define __CLOUD_STORAGE_LOGGING_HPP__

#include "utils/log/LogStream.hpp"
#include "utils/date/TimeStamp.hpp"

using namespace flkeeper::date;

namespace flkeeper {

namespace log {

class Logger {
public:
  // log日志级别
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS
  };

  // @brief 编译时计算源文件名字
  class SourceFile {
  public:
    template<int N>
    SourceFile(const char (&arr)[N]):data_(arr), size_(N-1) {
      const char *slash = strrchr(data_, '/');
      if(slash) {
        data_ = slash+1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char *filename) : data_(filename) {
      const char *slash = strrchr(filename, '/');
      if(slash) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *name);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream& stream() { return impl_.stream_; }

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char *msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc f);
  static void setFlush(FlushFunc f);

private:
  class Impl {
  public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile &file, int line);

    void formatTime();
    void finish();

    TimeStamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  Impl impl_;
};

extern Logger::LogLevel g_logLevel;
const char *strerror_tl(int savedErrno);

#define LOG_TRACE                                                              \
  if (flkeeper::log::Logger::logLevel() <= flkeeper::log::Logger::TRACE)       \
  flkeeper::log::Logger(__FILE__, __LINE__, flkeeper::log::Logger::TRACE,      \
                        __func__)                                              \
      .stream()
#define LOG_DEBUG                                                              \
  if (flkeeper::log::Logger::logLevel() <= flkeeper::log::Logger::DEBUG)       \
  flkeeper::log::Logger(__FILE__, __LINE__, flkeeper::log::Logger::DEBUG,      \
                        __func__)                                              \
      .stream()
#define LOG_INFO                                                               \
  if (flkeeper::log::Logger::logLevel() <= flkeeper::log::Logger::INFO)        \
  flkeeper::log::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN                                                               \
  flkeeper::log::Logger(__FILE__, __LINE__, flkeeper::log::Logger::WARN)       \
      .stream()
#define LOG_ERROR                                                              \
  flkeeper::log::Logger(__FILE__, __LINE__, flkeeper::log::Logger::ERROR)      \
      .stream()
#define LOG_FATAL                                                              \
  flkeeper::log::Logger(__FILE__, __LINE__, flkeeper::log::Logger::FATAL)      \
      .stream()
#define LOG_SYSERR flkeeper::log::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL flkeeper::log::Logger(__FILE__, __LINE__, true).stream()

}  // namespace log

}  // namespace flkeeper

#endif
