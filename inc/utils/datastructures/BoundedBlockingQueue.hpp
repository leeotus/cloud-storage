#ifndef __CLOUD_STORAGE_BOUNDEDBLOCKINGQUEUE_HPP__
#define __CLOUD_STORAGE_BOUNDEDBLOCKINGQUEUE_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/thread/PCondition.hpp"
#include "utils/thread/PMutex.hpp"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace flkeeper {

/**
 * @brief 有界阻塞队列
 * @note
 * 1. 线程安全的队列实现
 * 2. 支持多生产者多消费者
 * 3. 队列为空的时候，take操作会阻塞
 * 4. 队列满的时候，put操作会阻塞
 * 5. 使用环形缓冲区实现，固定容量
 * 6. 使用两个条件变量来实现同步
 * @tparam T 队列的元素
 */
template <typename T>
class BoundedBlockingQueue : NonCopyable {
public:
  /**
     * @brief 构造函数
     * @param maxSize 队列最大容量
     */
    explicit BoundedBlockingQueue(size_t maxSize)
        : mutex_(),
          notEmpty_(mutex_),
          notFull_(mutex_),
          queue_(maxSize)
    {
    }

    /**
     * @brief 将元素放入队列
     * @param x 要放入的元素
     * @note 如果队列已满，会阻塞到有空间可用
     */
    void put(const T& x)
    {
        thread::PMutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(x);
        notEmpty_.notifyOne(); // 唤醒等待的消费者
    }

    /**
     * @brief 将元素放入队列（移动语义版本）
     * @param x 要放入的元素
     */
    void put(T&& x)
    {
        thread::PMutexLockGuard lock(mutex_);
        while (queue_.full())
        {
            notFull_.wait();
        }
        assert(!queue_.full());
        queue_.push_back(std::move(x));
        notEmpty_.notifyOne();
    }

    /**
     * @brief 从队列中取出一个元素
     * @return 队首元素
     * @note 如果队列为空，会阻塞到有元素可取
     */
    T take()
    {
        thread::PMutexLockGuard lock(mutex_);
        while (queue_.empty())
        {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notifyOne(); // 唤醒等待的生产者
        return front;
    }

    /**
     * @brief 判断队列是否为空
     * @return true表示队列为空
     */
    bool empty() const
    {
        thread::PMutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief 判断队列是否已满
     * @return true表示队列已满
     */
    bool full() const
    {
        thread::PMutexLockGuard lock(mutex_);
        return queue_.full();
    }

    /**
     * @brief 获取队列中元素个数
     * @return 元素个数
     */
    size_t size() const
    {
        thread::PMutexLockGuard lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief 获取队列容量
     * @return 队列容量
     */
    size_t capacity() const
    {
        thread::PMutexLockGuard lock(mutex_);
        return queue_.capacity();
    }
private:
  mutable thread::PMutex mutex_;    // 互斥锁
  thread::PCondition notEmpty_;     // 非空条件变量
  thread::PCondition notFull_;      // 非满条件变量
  boost::circular_buffer<T> queue_;     // 环形缓冲区
};

} // namespace flkeeper

#endif
