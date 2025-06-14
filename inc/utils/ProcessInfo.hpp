#ifndef __CLOUD_STORAGE_PROCESSINFO_HPP__
#define __CLOUD_STORAGE_PROCESSINFO_HPP__

#include "utils/Types.hpp"
#include "utils/datastructures/FKString.hpp"
#include "utils/date/TimeStamp.hpp"
#include <vector>
#include <sys/types.h>

using std::string;
using namespace flkeeper::date;

namespace flkeeper {

namespace info {

  pid_t pid();
  string pidString();
  uid_t uid();
  string username();
  uid_t euid();
  TimeStamp startTime();
  DFLK_INT64 clockTicksPerSec();
  DFLK_INT32 pageSize();
  bool isDebugBuild();

  string hostName();
  string procName();
  FKString procName(const string &stat);

  // read /proc/self/status
  string procStatus();

  // read /proc/self/stat
  string procStat();

  // read /proc/self/task/tid/stat
  string threadStat();

  // readlink /proc/self/exe
  string exePath();

  int openedFiles();
  int maxOpenFiles();

  struct CpuTime {
    DFLK_FP userSeconds;
    DFLK_FP systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) {}
    ~CpuTime() = default;

    double total() const { return userSeconds + systemSeconds; }
  };

  CpuTime cpuTime();

  int numThreads();

  std::vector<pid_t> threads();
}   // namespace info

}  // namespace flkeeper

#endif
