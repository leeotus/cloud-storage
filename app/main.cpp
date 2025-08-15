#include "network/EventLoop.hpp"
#include "network/TcpClient.hpp"
#include "network/TcpConnection.hpp"
#include "network/http/HttpContext.hpp"
#include "network/http/HttpRequest.hpp"
#include "network/http/HttpResponse.hpp"
#include "network/http/HttpServer.hpp"
#include "utils/log/Logging.hpp"
#include "utils/thread/ThreadPool.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <mysql/mysql.h>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include "FileUploadContext.hpp"
#include "FileDownloadContext.hpp"
#include "HttpUploader.hpp"

using namespace flkeeper;
using namespace flkeeper::network;
using namespace flkeeper::thread;
using namespace flkeeper::log;
using json = nlohmann::json;
namespace fs = std::experimental::filesystem;

int main() {
  Logger::setLogLevel(Logger::INFO);
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8080), "http-upload-test");

  // 创建HTTP处理器
  auto handler = std::make_shared<HttpUploadHandler>(4);

  // 设置连接回调
  server.setConnectionCallback(
      [handler](const TcpConnectionPtr &conn) { handler->onConnection(conn); });

  // 设置HTTP回调
  server.setHttpCallback([handler](const TcpConnectionPtr &conn,
                                   HttpRequest &req, HttpResponse *resp) {
    return handler->onRequest(conn, req, resp);
  });

  server.setThreadNum(0);
  server.start();
  std::cout << "HTTP upload server is running on port 8080..." << std::endl;
  std::cout << "Please visit http://localhost:8080" << std::endl;
  loop.loop();
  return 0;
}
