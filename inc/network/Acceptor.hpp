#ifndef __CLOUD_STORAGE_ACCEPTOR_HPP__
#define __CLOUD_STORAGE_ACCEPTOR_HPP__

#include <functional>
#include "utils/NonCopyable.hpp"
#include "network/Channel.hpp"
#include "network/Socket.hpp"

namespace flkeeper {
namespace network {

class EventLoop;
class InetAddress;

///
/// Acceptor类用于接受TCP新连接
/// 使用非阻塞socket + Channel实现
/// 支持优雅关闭和防止文件描述符耗尽
///
class Acceptor : NonCopyable {
public:
  // 新连接回调函数类型
  // 参数1: 新连接的文件描述符
  // 参数2: 对端地址
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress &)>;

  // 构造函数
  // loop: 所属的事件循环
  // listenAddr: 监听地址
  // reuseport: 是否启用端口复用
  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
  ~Acceptor();

  // 设置新连接回调函数
  // 当有新连接到达时会调用此回调函数
  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  }

  // 返回是否正在监听
  bool listening() const { return listening_; }

  // 开始监听
  // 此函数必须在设置回调函数之后调用
  void listen();

private:
  // 处理新连接到达
  // 由Channel回调
  void handleRead();

  EventLoop *loop_;       // Acceptor所属的事件循环
  Socket acceptSocket_;   // 监听socket
  Channel acceptChannel_; // 监听Channel,用于观察监听socket上的可读事件
  NewConnectionCallback newConnectionCallback_; // 新连接回调函数
  bool listening_;                              // 是否正在监听
  int idleFd_; // 空闲的文件描述符，用于防止文件描述符耗尽
};

} // namespace network
} // namespace flkeeper

#endif
