#ifndef __CLOUD_STORAGE_THREADPOOL_HPP__
#define __CLOUD_STORAGE_THREADPOOL_HPP__

#include "utils/thread/PCondition.hpp"
#include "utils/thread/PMutex.hpp"
#include "utils/thread/Thread.hpp"
#include "utils/Types.hpp"
#include "utils/NonCopyable.hpp"

#include <deque>
#include <vector>
#include <memory>
#include <functional>

namespace flkeeper {

namespace thread {

using std::string;

/**
 * @brief 线程池类
 * @note
 * 1. 固定数量的线程
 * 2. 任务队列
 * 3. 支持优雅的关闭
 * 4. 支持自定义任务类型
 */
class ThreadPool : NonCopyable {
public:
  using Task = std::function<void()>;

  explicit ThreadPool(const string &nameArg = string("ThreadPool"));
  ~ThreadPool();

  // @brief 设置线程池的大小
  void setThreadSize(int numThread) { threadSize_ = numThread; }

  // @brief 设置任务队列的大小
  void setMaxQueueSize(size_t maxSize) { maxQueueSize_ = maxSize; }

  // @brief 启动线程池
  void start(int numThreads = 4);

  // @brief 停止线程池
  void stop();

  /**
   * @brief 添加新的任务到内存池中
   * @param task 任务函数
   */
  void run(Task task);

  // @brief 获取任务队列的大小
  size_t queeuSize() const;

private:
  // @brief 获取一个带执行的任务
  // @return Task 任务函数
  Task take();

  // @brief 线程执行函数
  void runInThread();

  // @brief 检查任务队列是否已经满了
  bool isFull() const;

  const string name_;   // 线程池的名称

  // @brief 互斥锁
  mutable PMutex mutex_;

  // @brief 条件变量，用于任务队列非空条件
  PCondition notEmpty_;

  // @brief 条件变量，用于任务队列非满条件
  PCondition notFull_;

  // @brief 线程池是否运行的标志
  bool running_;

  // @brief 最大任务队列大小
  size_t maxQueueSize_;

  // @brief 线程池大小
  int threadSize_;

  // @brief 任务队列
  std::deque<Task> queue_;

  // @brief 线程worker列表
  std::vector<std::unique_ptr<Thread>> threads_;
};

}   // namespace thread

}   // namespace flkeeper

#endif
