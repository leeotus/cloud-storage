#include "utils/thread/ThreadPool.hpp"
#include "utils/FKException.hpp"

namespace flkeeper {

namespace thread {

ThreadPool::ThreadPool(const string &name)
    : name_(name), mutex_(), notEmpty_(mutex_), notFull_(mutex_),
      running_(false), maxQueueSize_(0), threadSize_(0) {}

ThreadPool::~ThreadPool() {
  if(running_) {
    stop();
  }
}

void ThreadPool::start(int numThreads) {
  assert(!running_);
  running_ = true;

  threadSize_ = numThreads;

  // 创建线程
  threads_.reserve(threadSize_);
  for(int i=0;i<threadSize_;++i) {
    char id[32];
    snprintf(id, sizeof(id), "%d", i+1);
    threads_.emplace_back(
        new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));
    threads_[i]->start();
  }

  // 单线程模式，启动一个线程执行任务
  if(threadSize_ == 0 && !threads_.empty()) {
    threads_[0]->start();
  }
}

void ThreadPool::stop() {
  {
    PMutexLockGuard lock(mutex_);
    running_ = false;
    notEmpty_.notifyAll();
    notFull_.notifyAll();
  }

  for(auto &thr : threads_) {
    thr->join();
  }
}

size_t ThreadPool::queeuSize() const {
  PMutexLockGuard lock(mutex_);
  return queue_.size();
}

void ThreadPool::run(Task task) {
  if(threads_.empty()) {
    task();
  } else {
    PMutexLockGuard lock(mutex_);
    while(isFull() && running_) {
      notFull_.wait();
    }

    if(!running_) return;
    assert(!isFull());

    queue_.push_back(std::move(task));
    notEmpty_.notifyOne();
  }
}

ThreadPool::Task ThreadPool::take() {
  PMutexLockGuard lock(mutex_);
  // 等待任务队列非空
  while(queue_.empty() && running_) {
    notEmpty_.wait();
  }

  Task task;
  if(!queue_.empty()) {
    task = queue_.front();
    queue_.pop_front();
    if(maxQueueSize_ > 0) {
      notFull_.notifyOne();
    }
  }
  return task;
}

void ThreadPool::runInThread()
{
    try
    {
        if (threadSize_ == 0)
        {
            // 单线程模式
            while (running_)
            {
                Task task(take());
                if (task)
                {
                    task();
                }
            }
        }
        else
        {
            // 多线程模式
            while (running_)
            {
                Task task(take());
                if (task)
                {
                    task();
                }
            }
        }
    }
    catch (const FKException& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTraceInfo());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}

bool ThreadPool::isFull() const {
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

}   // namespace thread
}  // namespace flkeeper
