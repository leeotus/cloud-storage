#ifndef __CLOUD_STORGAE_BLOCKINGQUEUE_HPP__
#define __CLOUD_STORGAE_BLOCKINGQUEUE_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/thread/PCondition.hpp"
#include "utils/thread/PMutex.hpp"
#include <deque>
#include <assert.h>

namespace flkeeper {

/**
 * @brief 无界阻塞队列
 * @note
 * 1. 线程安全的队列实现
 * 2. 支持多生产者多消费者
 * 3. 队列为空的时候，take操作会阻塞
 * 4. 使用互斥锁和条件变量实现同步
 * 5. 无容量限制
 *
 * @tparam T 元素类型
 */
template<typename T>
class BlockingQueue: NonCopyable {
public:
  using queue_type = std::deque<T>;

  BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {}

  /**
   * @brief 将元素放入队列
   * @param x 要放入的元素
   */
  void put(const T& x) {
    thread::PMutexLockGuard g(mutex_);
    queue_.push_back(x);
    notEmpty_.notifyOne();
  }
  void put(T &&x) {
    thread::PMutexLockGuard g(mutex_);
    queue_.push_back(std::move(x));
    notEmpty_.notifyOne();
  }

  /**
   * @brief 从队列中取出一个元素
   * @note 如果队列为空则会阻塞,直到有元素被放入队列中
   * @return T 返回队列首的元素
   */
  T take() {
    thread::PMutexLockGuard g(mutex_);
    while(queue_.empty()) {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    // == T front = std::move(queue_.front());
    queue_.pop_front();
    return front;
  }

  /**
   * @brief 将队列中所有元素放入指定容器
   * @return queue_type 目标容器
   * @note 非阻塞操作，如果队列为空，目标容器也为空
   */
  queue_type drain() {
    queue_type result;
    {
      thread::PMutexLockGuard g(mutex_);
      result = std::move(queue_);
      assert(queue_.empty());
    }
    return result;
  }

  // @brief 获取队列中元素的个数
  size_t size() const {
    thread::PMutexLockGuard g(mutex_);
    return queue_.size();
  }

private:
  mutable thread::PMutex mutex_;      // 互斥锁
  thread::PCondition notEmpty_;       // 非空条件变量
  queue_type queue_;                  // 底层队列容器
};

}   // namespace flkeeper

#endif
