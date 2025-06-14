#include "utils/thread/CountDownLatch.hpp"

namespace flkeeper {

namespace thread {

CountDownLatch::CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count) {}

void CountDownLatch::wait() {
  PMutexLockGuard lock(mutex_);
  while(count_ > 0) {
    condition_.wait();
  }
}

void CountDownLatch::countDown() {
  PMutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0) {
    condition_.notifyAll();
  }
}

int CountDownLatch::getCount() const {
  PMutexLockGuard lock(mutex_);
  return count_;
}

}   // namespace thread

}   // namespace flkeeper
