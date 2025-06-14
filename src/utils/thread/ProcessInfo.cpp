#include "utils/ProcessInfo.hpp"
#include "utils/FileUtil.hpp"
#include "utils/thread/CurrentThread.hpp"

#include <algorithm>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pwd.h>
#include <time.h>

#include <sys/times.h>
#include <sys/resource.h>

namespace flkeeper {

__thread int t_numOpenedFiles = 0;
int fdDirFilter(const struct dirent* d){
  if(::isdigit(d->d_name[0])) {
    ++t_numOpenedFiles;
  }
  return 0;
}

__thread std::vector<pid_t> *t_pids = nullptr;
int taskDirFilter(const struct dirent *d) {
  if(::isdigit(d->d_name[0])) {
    t_pids->push_back(atoi(d->d_name));
  }
  return 0;
}

int scanDir(const char *dirpath, int (*filter)(const struct dirent*)) {
  struct dirent **namelist = nullptr;
  int res = ::scandir(dirpath, &namelist, filter, alphasort);
  assert(namelist == nullptr);
  return res;
}

TimeStamp g_startTime = TimeStamp::now();
DFLK_INT64 g_clockTicks = static_cast<DFLK_INT64>(::sysconf(_SC_CLK_TCK));
DFLK_INT32 g_pageSize = static_cast<DFLK_INT32>(::sysconf(_SC_PAGE_SIZE));

pid_t info::pid() {
  return ::getpid();
}

string info::pidString() {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", pid());
  return buf;
}

uid_t info::uid() {
  return ::getuid();
}

/**
 * @brief 借助当前用户的uid来获取对应的用户名
 */
string info::username() {
  struct passwd pwd;
  struct passwd* result = nullptr;
  char buf[8192];
  const char *name = "unknownuser";

  getpwuid_r(uid(), &pwd, buf, sizeof(buf), &result);
  if(result) {
    name = pwd.pw_name;
  }
  return name;
}

// @brief 用于获取当前进程的有效用户id
uid_t info::euid() {
  return ::geteuid();
}

TimeStamp info::startTime() {
  return g_startTime;
}

DFLK_INT64 info::clockTicksPerSec() {
  return g_clockTicks;
}

DFLK_INT32 info::pageSize() {
  return g_pageSize;
}

bool info::isDebugBuild() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

string info::hostName() {
  char buf[256];
  if(::gethostname(buf, sizeof(buf)) == 0) {
    buf[sizeof(buf)-1] = '\0';
    return buf;
  } else {
    return "unknownhost";
  }
}

string info::procName() {
  return procName(procStat()).as_string();
}

FKString info::procName(const string& stat) {
  FKString name;
  size_t lp = stat.find('(');
  size_t rp = stat.find(')');
  if(lp != string::npos && rp != string::npos && lp < rp) {
    name.set(stat.data()+lp+1, static_cast<int>(rp-lp-1));
  }
  return name;
}

string info::procStatus() {
  string res;
  readFile("/proc/self/status", &res);
  return res;
}

string info::procStat() {
  string res;
  readFile("/proc/self/stat", &res);
  return res;
}

string info::threadStat() {
  char buf[64];
  snprintf(buf, sizeof(buf), "/proc/self/task/%d/stat", thread::tid());
  string res;
  readFile(buf, &res);
  return res;
}

string info::exePath() {
  string res;
  char buf[1024];
  ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf));
  if(n > 0) {
    res.assign(buf, n);
  }
  return res;
}

int info::openedFiles()
{
    t_numOpenedFiles = 0;
    scanDir("/proc/self/fd", fdDirFilter);
    return t_numOpenedFiles;
}

int info::maxOpenFiles()
{
    struct rlimit rl;
    if (::getrlimit(RLIMIT_NOFILE, &rl))
    {
        return openedFiles();
    }
    else
    {
        return static_cast<int>(rl.rlim_cur);
    }
}

info::CpuTime info::cpuTime()
{
    info::CpuTime t;
    struct tms tms;
    if (::times(&tms) >= 0)
    {
        const double hz = static_cast<double>(clockTicksPerSec());
        t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
        t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
    }
    return t;
}

int info::numThreads()
{
    int result = 0;
    string status = procStatus();
    size_t pos = status.find("Threads:");
    if (pos != string::npos)
    {
        result = ::atoi(status.c_str() + pos + 8);
    }
    return result;
}

std::vector<pid_t> info::threads()
{
    std::vector<pid_t> result;
    t_pids = &result;
    scanDir("/proc/self/task", taskDirFilter);
    t_pids = NULL;
    std::sort(result.begin(), result.end());
    return result;
}

}   // namespace flkeeper
