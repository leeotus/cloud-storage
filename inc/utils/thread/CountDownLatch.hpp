#ifndef __CLOUD_STORAGE_COUNTDOWNLATCH_HPP__
#define __CLOUD_STORAGE_COUNTDOWNLATCH_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/thread/PCondition.hpp"
#include "utils/thread/PMutex.hpp"
#include "utils/Types.hpp"

namespace flkeeper::thread {

/**
 * @brief 倒计时门闩类，用于线程同步
 * @note
 * 1. 可以用于确保线程的安全启动
 * 2. 可以用于等待多个线程完成某个事件
 * 3. 基于条件变量和互斥锁实现
 */
class CountDownLatch : NonCopyable {
public:
  explicit CountDownLatch(int count);

  // @brief 等待计数值变为0
  void wait();

  // @brief 计数值减1
  void countDown();

  // @brief 获取当前计数值
  int getCount() const;

private:
  mutable PMutex mutex_;    // 互斥锁
  PCondition condition_;    // 条件变量
  int count_;                       // 计数值
};

} // namespace flkeeper::thread



#endif
