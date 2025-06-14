#include "utils/thread/CurrentThread.hpp"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>

namespace flkeeper {

namespace thread {

  // 初始化线程局部存储变量
  __thread int t_cachedTid = 0;
  __thread char t_tidString[32];
  __thread int t_tidStringLength = 6;
  __thread const char *t_threadName = "unknown";

  /**
   * @brief 调用SYS_gettid获取线程的真实ID
   * @return pid_t 线程的真实ID
   */
  pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
  }

  void cacheTid() {
    if(t_cachedTid == 0) {
      // 没有缓存当前的线程id
      t_cachedTid = gettid();

      // 将线程ID转换为字符串形式
      t_tidStringLength =
          snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
    }
  }

  // @note ::getpid() 调用是获取当前进程的进程ID, 主线程的线程id与进程id相同
  bool isMainThread() {
    return tid() == ::getpid();
  }

  void sleepUsec(int usec) {
    struct timeval tv = {0,0};
    tv.tv_sec = static_cast<time_t>(usec / 1000000);
    tv.tv_usec = static_cast<suseconds_t>(usec % 1000000);

    // 使用select函数阻塞当前线程知道指定时间过去
    ::select(0, NULL, NULL, NULL, &tv);
  }

} // namespace thread

} // namespace flkeeper
