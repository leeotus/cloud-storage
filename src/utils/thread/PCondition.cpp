#include "utils/thread/PCondition.hpp"
#include <time.h>

namespace flkeeper {

namespace thread {

PCondition::~PCondition() {
  pthread_cond_destroy(&pcond_);
}

void PCondition::wait() {
  pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
}

void PCondition::notifyOne() {
  pthread_cond_signal(&pcond_);
}

void PCondition::notifyAll() {
  pthread_cond_broadcast(&pcond_);
}

bool PCondition::waitFor(DFLK_INT64 sec) {
struct timespec abstime;
clock_gettime(CLOCK_REALTIME, &abstime);

const DFLK_INT64 nano = 1000000000;
DFLK_INT64 nanoseconds = static_cast<DFLK_INT64>(sec * nano);

abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) /
                                      nano);
abstime.tv_nsec =
    static_cast<long>((abstime.tv_nsec + nanoseconds) % nano);

return ETIMEDOUT ==
       pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}

} // namespace thread

} // namespace flkeeper
