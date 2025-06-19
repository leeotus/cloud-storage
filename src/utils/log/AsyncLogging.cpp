#include "utils/log/AsyncLogging.hpp"
#include "utils/date/TimeStamp.hpp"
#include "utils/log/LogFile.hpp"

namespace flkeeper::log {

AsyncLogging::AsyncLogging(string basename, off_t rollSize, int flushInterval)
    : flushInterval_(flushInterval), running_(false), basename_(basename),
      rollSize_(rollSize), thread_([this] { threadFunc(); }, "Logging"),
      latch_(1), mutex_(), cond_(mutex_), currentBuffer_(new Buffer),
      nextBuffer_(new Buffer), buffers_() {
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char *logline, int len) {
  thread::PMutexLockGuard lock(mutex_);

  if (currentBuffer_->avail() > len) {
    // 当前缓冲区足够，则直接加入缓冲区 NOTE: 这是最常见的情况
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(currentBuffer_));

    if (nextBuffer_) {
      // 如果当前缓冲区不够用，则将备用缓冲区的权限交给当前缓冲区
      // @note 后续需要使用 `nextBuffer_ = std::move(somebuffer);`的方式来为nextBuffer_分配缓冲区
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      currentBuffer_.reset(new Buffer); // Rarely happens
    }
    currentBuffer_->append(logline, len);
    cond_.notifyOne();
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  latch_.countDown();

  // 创建日志对象和两个缓冲区，用于后续的日志写入和交换
  LogFile output(basename_, rollSize_, false);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();

  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);

  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      thread::PMutexLockGuard lock(mutex_);
      if (buffers_.empty()) // unusual usage!
      {
        cond_.waitFor(flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());

    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log messages at %s, %zd larger buffers\n",
               date::TimeStamp::now().toFormattedString().c_str(),
               buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto &buffer : buffersToWrite) {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}

} // namespace flkeeper::log
