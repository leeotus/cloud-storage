#include "utils/thread/PMutex.hpp"

namespace flkeeper {

namespace thread {

PMutex::PMutex():holder_(0) {
  // 初始化互斥锁
  pthread_mutex_init(&mutex_, nullptr);
}

PMutex::~PMutex() {
  assert(holder_ == 0);   // 确保没有任何其他线程持有该互斥锁
  pthread_mutex_destroy(&mutex_);
}

bool PMutex::isLockedByCurrentThread() const {
  return holder_ == gettid();
}

void PMutex::lock() {
  pthread_mutex_lock(&mutex_);
  assignHolder();
}

void PMutex::unlock() {
  pthread_mutex_unlock(&mutex_);
  unassignHolder();
}

pthread_mutex_t* PMutex::getPthreadMutex() {
  return &mutex_;
}

void PMutex::assignHolder() {
  holder_ = gettid();
}

void PMutex::unassignHolder() {
  holder_ = 0;
}

void PMutex::assertLocked() const {
  assert(isLockedByCurrentThread());
}

}   // namespace thread

}   // namespace flkeeper
