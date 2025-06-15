#ifndef __CLOUD_STORAGE_HTTPSERVER_HPP__
#define __CLOUD_STORAGE_HTTPSERVER_HPP__

#include "HttpContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "network/TcpServer.hpp"

namespace flkeeper {
namespace network {

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
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                 TimeStamp receiveTime);
  bool onRequest(const TcpConnectionPtr &, HttpRequest &);

  TcpServer server_;
  HttpCallback httpCallback_;
}; // class HttpServer

} // namespace network
} // namespace flkeeper

#endif
