#include "network/TcpClient.hpp"
#include "network/Connector.hpp"
#include "network/EventLoop.hpp"
#include "network/InetAddress.hpp"
#include "network/SocketsOps.hpp"
#include "network/TcpConnection.hpp"
#include "utils/log/Logging.hpp"

#include <functional>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

using namespace std::placeholders;

namespace flkeeper {
namespace network {

void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector) { connector->stop(); }

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(loop), name_(nameArg),
      connector_(std::make_shared<Connector>(loop, serverAddr)), retry_(false),
      connect_(true), nextConnId_(1) {
  // 设置连接器的新连接回调函数
  connector_->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, _1));

  LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);

  TcpConnectionPtr conn;
  bool unique = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }

  if (conn) {
    // 重置连接的回调函数
    CloseCallback cb = std::bind(&network::removeConnection, loop_, _1);
    loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_->stop();
    loop_->runAfter(1, std::bind(&network::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();

  struct sockaddr_in peeraddr;
  ::memset(&peeraddr, 0, sizeof peeraddr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
  if (::getpeername(sockfd, reinterpret_cast<struct sockaddr *>(&peeraddr),
                    &addrlen) < 0) {
    LOG_ERROR << "TcpClient::newConnection - getpeername failed";
    return;
  }
  InetAddress peerAddr(peeraddr);

  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  struct sockaddr_in localaddr;
  ::memset(&localaddr, 0, sizeof localaddr);
  addrlen = static_cast<socklen_t>(sizeof localaddr);
  if (::getsockname(sockfd, reinterpret_cast<struct sockaddr *>(&localaddr),
                    &addrlen) < 0) {
    LOG_ERROR << "TcpClient::newConnection - getsockname failed";
    return;
  }
  InetAddress localAddr(localaddr);

  // 创建TcpConnection对象
  TcpConnectionPtr conn(
      new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));

  {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

  if (retry_ && connect_) {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toIpPort();
    connector_->restart();
  }
}

} // namespace network
}   // namespace flkeeper
