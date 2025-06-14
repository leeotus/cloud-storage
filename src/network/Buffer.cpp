#include "network/Buffer.hpp"
#include <errno.h>
#include <sys/uio.h>
#include <algorithm>

namespace flkeeper {
namespace network {

const char Buffer::kCRLF[] = "\r\n";

const char *Buffer::findCRLF() const {
  const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
  return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findCRLF(const char *start) const {
  assert(peek() <= start);
  assert(start <= beginWrite());
  const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
  return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findEOL() const {
  const void *eol = memchr(peek(), '\n', readableBytes());
  return static_cast<const char *>(eol);
}

const char* Buffer::findEOL(const char *start) const {
  assert(peek() <= start);
  assert(start <= beginWrite());
  const void *eol = memchr(start, '\n', beginWrite() - start);
  return static_cast<const char*>(eol);
}

size_t Buffer::readableBytes() const {
  return writeIndex_ - readIndex_;
}

size_t Buffer::writableBytes() const {
  return buffer_.size() - writeIndex_;
}

size_t Buffer::prependableBytes() const {
  return readIndex_;
}

void Buffer::checkWritableSpace(size_t len) {
  if(writableBytes() < len) {
    makeSpace(len);
  }
  assert(writableBytes() >= len);
}

void Buffer::swap(Buffer& rhs)
{
  buffer_.swap(rhs.buffer_);
  std::swap(readIndex_, rhs.readIndex_);
  std::swap(writeIndex_, rhs.writeIndex_);
}

const char *Buffer::peek() const {
  return begin() + readIndex_;
}

void Buffer::append(const char *data, size_t len) {
  checkWritableSpace(len);
  std::copy(data, data+len, beginWrite());
  hasWritten(len);
}

void Buffer::append(const std::string& str) {
  append(str.data(), str.size());
}

void Buffer::prepend(const void* data, size_t len) {
  assert(len <= prependableBytes());
  readIndex_ -= len;
  const char* d = static_cast<const char*>(data);
  std::copy(d, d+len, begin()+readIndex_);
}

void Buffer::shrink(size_t reserve) {
  Buffer other;
  other.checkWritableSpace(readableBytes() + reserve);
  other.append(peek(), readableBytes());
  swap(other);
}

void Buffer::makeSpace(size_t len) {
  // or: if(writableBytes() < len)
  if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writeIndex_ + len);
  } else {
    assert(kCheapPrepend < readIndex_);
    auto readable = readableBytes();
    std::copy(begin() + readIndex_, begin() + writeIndex_,
              begin() + kCheapPrepend);
    readIndex_ = kCheapPrepend;
    writeIndex_ = readIndex_ + readable;
    assert(readable == readableBytes());
  }
}

char* Buffer::beginWrite() { return begin()+writeIndex_; }
const char*Buffer::beginWrite() const { return begin()+writeIndex_; }

ssize_t Buffer::readFd(int fd, int *savedErrno) {
  // 栈上的额外缓冲区
  char extrabuf[BUFFER_LEN];
  struct iovec vec[2];
  const size_t writable = writableBytes();

  // 第一块缓冲区是Buffer中的writable空间
  vec[0].iov_base = begin() + writeIndex_;
  vec[0].iov_len = writable;
  // 第二块缓冲区是栈上的额外缓冲区
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  // 如果Buffer空间足够大，就不使用额外的缓冲区
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iovcnt);

  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    // 数据完全写入Buffer中
    writeIndex_ += n;
  } else {
    // Buffer写满了，额外的数据写入了extrabuf，需要append
    writeIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }

  return n;
}

} // namespace network
} // namespace flkeeper
