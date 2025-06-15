#include "utils/thread/Thread.hpp"
#include "utils/thread/CurrentThread.hpp"
#include "utils/FKException.hpp"
#include "utils/log/Logging.hpp"

#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace flkeeper {

namespace thread {

/**
 * @brief 线程数据结构，用于在线程函数中传递数据
 */
struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;        // 线程函数
    std::string name_;           // 线程名称
    pid_t* tid_;           // 线程ID指针
    CountDownLatch* latch_; // 用于确保线程安全启动的门闩

    ThreadData(ThreadFunc func,
              const std::string& name,
              pid_t* tid,
              CountDownLatch* latch)
        : func_(std::move(func)),
          name_(name),
          tid_(tid),
          latch_(latch)
    { }

    /**
     * @brief 运行线程函数
     */
    void runInThread()
    {
        *tid_ = tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;


        setName(name_.empty() ? "flkeeperThread" : name_.c_str());
        ::prctl(PR_SET_NAME, name());

        try
        {
            func_();
            setName("finished");
        }
        catch (const FKException& ex)
        {
            setName("crashed");
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTraceInfo());
            abort();
        }
        catch (const std::exception& ex)
        {
            setName("crashed");
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            setName("crashed");
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
    }
};

/**
 * @brief 线程入口函数
 */
void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}


// 初始化静态成员变量
AtomicInt32 numCreated_;

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(name),
      latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    int num = thread::numCreated_.incrementAndGet();
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;

    // 创建线程数据
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);

    // 创建线程
    if (pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
        LOG_SYSFATAL << "Failed in pthread_create";
    }
    else
    {
        // 等待线程启动完成
        latch_.wait();
        assert(tid_ > 0);
    }
}

void Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    if (pthread_join(pthreadId_, NULL))
    {
        LOG_SYSFATAL << "Failed in pthread_join";
    }
}

}   // namespace thread

} // namespace flkeeper
