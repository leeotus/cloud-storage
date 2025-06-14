#ifndef __CLOUD_STORAGE_CURRENTTHREAD_HPP__
#define __CLOUD_STORAGE_CURRENTTHREAD_HPP__

#include <stdint.h>

namespace flkeeper {

namespace thread {

  // @note 使用__thread修饰符声明的变量在每个线程中都是独立的
  // 这意味着每个线程都会有该变量的一个独立副本，互不干扰
  // 内部使用的线程局部存储变量
  extern __thread int t_cachedTid;    // 缓存的线程ID
  extern __thread char t_tidString[32];     // 线程ID的字符串表示
  extern __thread int t_tidStringLength;    // 线程ID字符串的长度
  extern __thread const char* t_threadName;     // 线程名称

  /**
   * @brief 缓存线程ID
   * 通过系统调用获取线程ID并缓存，避免频繁系统调用
   */
  void cacheTid();

  /**
   * @brief 获取线程ID
   * 如果还未缓存，则调用cacheTid()进行缓存
   * @return int  返回当前线程的ID
   */
  inline int tid() {
    if(__builtin_expect(t_cachedTid == 0, 0)) {
      cacheTid();
    }
    return t_cachedTid;
  }

  /**
   * @brief 设置当前线程的名称
   * @param name 线程名称
   */
  inline void setName(const char *name) {
    t_threadName = name;
  }

  // @brief 判断是否是主线程
  bool isMainThread();

  /**
   * @brief 获取线程ID的字符串表示
   * @return 返回线程ID的字符串
   */
  inline const char *tidString() {
    return t_tidString;
  }

  /**
   * @brief 获取线程名称
   * @return const char* 返回当前线程的名称，如果没有设置则返回"unknown";
   */
  inline const char *name() {
    return t_threadName;
  }

  /**
   * @brief 休眠
   * @param usec 休眠的时间，单位是微秒
   */
  void sleepUsec(int usec);
}   // namespace thread

} // namespace flkeeper

#endif
