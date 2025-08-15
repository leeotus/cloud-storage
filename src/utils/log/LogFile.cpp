#include "utils/log/LogFile.hpp"
#include "utils/ProcessInfo.hpp"
#include <string>

namespace flkeeper::log {

LogFile::LogFile(const string &basename, off_t rollSize, bool threadSafe,
                 int flushInterval, int checkEveryN)
    : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval),
      checkEveryN_(checkEveryN), count_(0),
      mutex_(threadSafe ? new thread::PMutex : nullptr), startOfPeriod_(0),
      lastRoll_(0), lastFlush_(0) {
  // 不用写入路径
  assert(basename.find('/') == string::npos);
  rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char *logline, int len) {
  if (mutex_) {
    thread::PMutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  } else {
    append_unlocked(logline, len);
  }
}

void LogFile::flush() {
  if (mutex_) {
    thread::PMutexLockGuard lock(*mutex_);
    file_->flush();
  } else {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char *logline, int len) {
  file_->append(logline, len);

  // @note 如果已经写入的字节数超过滚动大小rollSize_,则需要分配一个新的文件
  if (file_->writtenBytes() > rollSize_) {
    rollFile();
  } else {
    ++count_;
    // @note count_是计数器, checkEveryN_是检查频率，计数器每次到达checkEveryN_进行一次判断
    if (count_ >= checkEveryN_) {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;

      if (thisPeriod_ != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        // RESEARCH: 可能会出现file->append()和file->flush提示出现的情况
        // @note 超过刷新时间之后将数据从缓冲区写入磁盘
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

bool LogFile::rollFile() {
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new AppendFile(filename)); // 创建一个新的文件
    return true;
  }
  return false;
}

string LogFile::getLogFileName(const string &basename, time_t *now) {
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm); // thread safe
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  filename += info::hostName();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", info::pid());
  filename += pidbuf;

  filename += ".log";
  return filename;
}

} // namespace flkeeper::log


