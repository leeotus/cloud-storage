#ifndef __CLOUD_STORAGE_THREAD_HPP__
#define __CLOUD_STORAGE_THREAD_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/datastructures/Atomic.hpp"
#include "utils/thread/CountDownLatch.hpp"

#include <functional>
#include <pthread.h>

namespace flkeeper {

namespace thread {

/**
 * @brief 线程类，封装pthread,提供RAII方式的线程管理
 * @note
 * 1. 使用std::function封装线程函数
 * 2. 使用RAII方式管理线程资源
 * 3. 提供线程计数功能
 * 4. 支持线程命名
 * 5. 使用CountDownLatch确保线程安全启动
 */
class Thread : NonCopyable {
public:
  // 线程的回调函数类型
  typedef std::function<void()> ThreadFunc;

  explicit Thread(ThreadFunc func, const std::string& name=std::string());
  ~Thread();

  // @brief 启动线程
  void start();

  // @brief 等待线程结束
  void join();

  // @brief 线程是否已经启动
  bool started() { return started_; }

  // @brief 线程是否已经加入
  bool joined() { return joined_; }

  // @brief 获取线程的id
  pid_t tid() const { return tid_; }

  // @brief 获取线程的名称
  const std::string &name() const { return name_; }

  // @brief 获取当前活跃线程数
  static int numCreated() { return numCreated_.get(); }

private:
  // @brief 设置默认线程名称
  void setDefaultName();

  bool started_;    // 是否已启动
  bool joined_;     // 是否已join
  pthread_t pthreadId_; // pthread线程ID
  pid_t tid_;     // 线程真实id
  ThreadFunc func_;   // 线程函数
  std::string name_;    // 线程名称
  CountDownLatch latch_;    // 用于确保线程安全启动

  static AtomicInt32 numCreated_;   // 已创建的线程数
};

}   // namespace thread

}   // namespace flkeeper

#endif
