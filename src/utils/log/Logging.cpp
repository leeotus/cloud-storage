#include "utils/log/Logging.hpp"

#include <cassert>
#include <string.h>
#include <stdio.h>
#include <errno.h>

namespace flkeeper {

namespace log {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

Logger::LogLevel g_logLevel;

Logger::LogLevel Logger::logLevel() { return g_logLevel; }

const char* strerror_tl(int savedErrno)
{
  // @note strerror_r 是一个线程安全的函数
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initialLogLevel() {
  return Logger::INFO;
}

const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
  "TRACE ",
  "DEBUG ",
  "INFO ",
  "WARN ",
  "ERROR ",
  "FATAL ",
};

void defaultOutput(const char *msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  (void)n;
}

// @brief 刷新数据，将数据输出
void defaultFlush() { fflush(stdout); }

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file,
                   int line)
    : time_(TimeStamp::now()), stream_(), level_(level), line_(line),
      basename_(file) {
  formatTime();
  stream_ << LogLevelName[level];
  if (savedErrno != 0) {
    stream_ << strerror_tl(savedErrno) << "(errno = " << savedErrno << ") ";
  }
}

void Logger::Impl::formatTime() {
  DFLK_INT64 microSecsSinceEpoch = time_.microSecondsSinceEpoch();
  time_t seconds =
      static_cast<time_t>(microSecsSinceEpoch / TimeStamp::kMicroSecondsPerSec);
  int microseconds = static_cast<DFLK_INT64>(microSecsSinceEpoch %
                                             TimeStamp::kMicroSecondsPerSec);
  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;
    struct tm tm_time;
    ::gmtime_r(&seconds, &tm_time);

    int len =
        snprintf(t_time, sizeof(t_time), "%04d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17);
    (void)len;
  }
  char buf[32];
  snprintf(buf, sizeof(buf), ".%06d ", microseconds);
  stream_ << t_time << buf;
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_.data_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer &buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) { g_output = out; }

void Logger::setFlush(FlushFunc flush) { g_flush = flush; }

} // namespace log

} // namespace flkeeper

