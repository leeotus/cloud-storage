#ifndef __CLOUD_STORAGE_PCONDITION_HPP__
#define __CLOUD_STORAGE_PCONDITION_HPP__

#include "utils/Types.hpp"
#include "utils/thread/PMutex.hpp"
#include "utils/NonCopyable.hpp"

#include <pthread.h>

namespace flkeeper {

namespace thread {

class PCondition : NonCopyable {
public:
  explicit PCondition(PMutex &mutex): mutex_(mutex) {
    pthread_cond_init(&pcond_, nullptr);
  }
  ~PCondition();

  /**
   * @brief 等待条件变量被触发
   */
  void wait();

  /**
   * @brief 等待指定秒数的时间
   * @param sec 时间，以秒作为单位
   * @todo
   */
  bool waitFor(DFLK_INT64 sec);

  /**
   * @brief 唤醒/通知一个条件变量
   */
  void notifyOne();

  void notifyAll();

private:
  PMutex &mutex_;             // 互斥锁
  pthread_cond_t pcond_;      // 条件变量
};

} //namespace thread

}   // namespace flkeeper

#endif
