#ifndef __CLOUD_STORAGE_PTHREAD_MUTEX_HPP__
#define __CLOUD_STORAGE_PTHREAD_MUTEX_HPP__

#include "utils/NonCopyable.hpp"

#include <assert.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>

namespace flkeeper {

namespace thread {

/**
 * @brief 获取当前线程的线程id
 */
inline pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

class PMutex: NonCopyable {
public:
  PMutex();
  ~PMutex();

  /**
   * @brief 判断互斥锁是否被当前线程所持有
   */
  bool isLockedByCurrentThread() const;

  // 互斥锁上锁
  void lock();

  // 互斥锁解锁
  void unlock();

  // 获取互斥锁
  pthread_mutex_t* getPthreadMutex();

  // 断言锁被当前线程所持有
  void assertLocked() const;

private:
  // 当前线程释放该锁后，将锁持有者(holder_)设置为空(0)
  void unassignHolder();

  // 当前锁被当前线程持有，设置锁持有者(holder_)为当前线程(gettid())
  void assignHolder();

  pthread_mutex_t mutex_;   // 互斥锁
  pid_t holder_;            // 当前持有该互斥锁的线程

  friend class PCondition;
};

// 以RAII的方式调用PMutex类
class PMutexLockGuard: NonCopyable {
public:
  explicit PMutexLockGuard(PMutex &mutex) : mutex_(mutex) {
    mutex_.lock();
  }

  ~PMutexLockGuard() {
    mutex_.unlock();
  }

private:
  PMutex &mutex_;
};

#define PMutexLockGuard(x) error "Missing guard object name"

} // namespace thread

} // namespace flkeeper

#endif
