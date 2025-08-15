#ifndef __CLOUD_STORGAE_ASYNCLOGGING_HPP__
#define __CLOUD_STORGAE_ASYNCLOGGING_HPP__

#include "utils/datastructures/BlockingQueue.hpp"
#include "utils/datastructures/BoundedBlockingQueue.hpp"
#include "utils/thread/PCondition.hpp"
#include "utils/thread/PMutex.hpp"
#include "utils/thread/Thread.hpp"
#include "utils/thread/CountDownLatch.hpp"
#include "utils/NonCopyable.hpp"
#include "LogStream.hpp"

#include <atomic>
#include <vector>

namespace flkeeper::log {

// @note 双缓冲异步日志
class AsyncLogging : NonCopyable {
public:
    /**
     * @brief 构造函数
     * @param basename 日志文件基本名
     * @param rollSize 滚动大小
     * @param flushInterval 刷新间隔，秒
     */
    AsyncLogging(std::string basename,
                off_t rollSize,
                int flushInterval = 3);
    ~AsyncLogging()
    {
        if (running_)
        {
            stop();
        }
    }

    /**
     * @brief 前端调用接口，写入日志
     * @note 前端可以通过此API向缓存区写入信息,后端通过线程来处理写入的日志
     */
    void append(const char* logline, int len);

    /**
     * @brief 启动日志线程
     */
    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    /**
     * @brief 停止日志线程
     */
    void stop()
    {
        running_ = false;
        cond_.notifyOne();
        thread_.join();
    }

private:
    /**
     * @brief 后端线程函数
     */
    void threadFunc();

    typedef FixedBuffer<largeBufferLen> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flushInterval_;        // 刷新间隔
    std::atomic<bool> running_;      // 运行标志
    const std::string basename_;          // 日志文件基本名
    const off_t rollSize_;          // 滚动大小
    thread::Thread thread_;        // 后端线程
    thread::CountDownLatch latch_; // 用于等待线程启动
    thread::PMutex mutex_;      // 互斥锁
    thread::PCondition cond_;       // 条件变量
    BufferPtr currentBuffer_;       // 当前缓冲区
    BufferPtr nextBuffer_;          // 预备缓冲区
    BufferVector buffers_;          // 待写入文件的缓冲区列表
};

} // namespace flkeeper::log


#endif
