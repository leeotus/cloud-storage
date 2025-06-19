#ifndef __CLOUD_STORAGE_HTTPCONTEXT_HPP__
#define __CLOUD_STORAGE_HTTPCONTEXT_HPP__

#include "network/Buffer.hpp"
#include "HttpRequest.hpp"
#include "utils/log/Logging.hpp"
#include "utils/Copyable.hpp"
#include "utils/date/TimeStamp.hpp"

#include <memory>
#include <unordered_map>

namespace flkeeper::network {

class HttpContext : public NonCopyable
{
 public:
  enum HttpRequestParseState
  {
    // 解析过程的不同状态
    kExpectRequestLine, // 期望解析请求行
    kExpectHeaders,     // 期望解析请求头部
    kExpectBody,        // 期望解析请求体
    kGotAll,            // 完成全部请求的解析
  };

  enum ParseResult
  {
    kError = -1,           // 解析出错
    kNeedMore = 0,         // 需要更多数据
    kHeadersComplete = 1, // 头部解析完成
    kGotRequest = 2       // 整个请求解析完成
  };

  HttpContext() :
      contentLength_(0),
      bodyReceived_(0),
      isChunked_(false)
  {
  }

  ~HttpContext()
  {
    LOG_INFO << "HttpContext destroyed";
    // customContext_.reset();
  }

  // default copy-ctor, dtor and assignment are fine

  // 返回false表示解析出错，true表示解析成功（包括需要更多数据和解析完成的情况）
  ParseResult parseRequest(Buffer* buf, TimeStamp receiveTime);

  bool gotAll() const
  { return state_ == kGotAll; }

   bool expectBody() const
  { return state_ == kExpectBody; }

  bool headersComplete() const
  { return state_ == kExpectBody || state_ == kGotAll; }

  size_t remainingLength() const
  { return contentLength_ - bodyReceived_; }

  bool isChunked() const
  { return isChunked_; }

  // @brief 重置
  void reset()
  {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
    contentLength_ = 0;
    bodyReceived_ = 0;
    isChunked_ = false;
    customContext_.reset();
  }

  // @brief 获取request
  const HttpRequest& request() const
  { return request_; }
  HttpRequest& request()
  { return request_; }

  // @brief 返回解析的状态
  HttpRequestParseState state() const { return state_; }

  template<typename T>
  std::shared_ptr<T> getContext() const {
    return std::static_pointer_cast<T>(customContext_);
  }

  void setContext(const std::shared_ptr<void>& context) {
    customContext_ = context;
  }

 private:
  // @brief 用于解析http请求行，提取请求方法、请求路径、查询数和HTTP版本信息
  bool processRequestLine(const char* begin, const char* end);

  // @brief 解析HTTP请求头部，将头部字段和对应的值存储在headers_成员中
  bool processHeaders(Buffer* buf);

  // @brief 用于处理HTTP请求体
  bool processBody(Buffer* buf);

  HttpRequestParseState state_ = kExpectRequestLine;
  HttpRequest request_;
  size_t contentLength_;  // 用于存储 Content-Length 的值
  size_t bodyReceived_;   // 已接收的 body 长度
  bool isChunked_;        // 是否为 chunked 传输
  std::shared_ptr<void> customContext_;  // 自定义上下文存储
};

} // namespace flkeeper::network


#endif
