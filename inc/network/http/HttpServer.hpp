#ifndef __CLOUD_STORAGE_HTTPSERVER_HPP__
#define __CLOUD_STORAGE_HTTPSERVER_HPP__

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "network/TcpServer.hpp"

namespace flkeeper::network {

/**
 * @brief 监听指定地址和端口，接收客户端的HTTP请求，解析请求内容，并根据用户设置
 * 的回调函数处理请求,最后返回HTTP响应
*/
class HttpServer {
public:
  using HttpCallback = std::function<bool(const TcpConnectionPtr &,
                                          HttpRequest &, HttpResponse *)>;

  HttpServer(EventLoop *loop, const InetAddress &listenAddr,
             const std::string &name);

  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }
  void setConnectionCallback(const ConnectionCallback &cb) {
    server_.setConnectionCallback(cb);
  }
  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }
  void start() { server_.start(); }

private:
  void onConnection(const TcpConnectionPtr &conn);

  /**
   * @brief 负责处理接收到的HTTP消息
   * @param conn 表示一个TCP连接的智能指针,用于与客户端进行通信
   * @param buf 指向Buffer对象的指针,用于缓存接收到的数据
   * @param receiveTime 接收消息的时间戳(i.e., 接收到该数据的时刻)
   */
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                 TimeStamp receiveTime);
  bool onRequest(const TcpConnectionPtr &, HttpRequest &);

  // 用于处理底层TCP连接和事件循环,HttpServer基于TcpServer实现，利用其
  // 提供的功能来监听和处理TCP连接
  TcpServer server_;

  // 用于处理HTTP请求,用户可以通过`setHttpCallback`方法来设置该回调函数,以实现自定义的请求处理逻辑
  HttpCallback httpCallback_;
}; // class HttpServer

} // namespace flkeeper::network

#endif
