#ifndef __CLOUD_STORAGE_INETADDRESS_HPP__
#define __CLOUD_STORAGE_INETADDRESS_HPP__

#include "utils/Copyable.hpp"
#include "utils/datastructures/FKString.hpp"
#include "utils/Types.hpp"

#include <netinet/in.h>
#include <string>

namespace flkeeper {

namespace network {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6 *addr);

typedef std::string& StringArg;

class InetAddress : public Copyable {
public:
  /**
   * @brief Construct a new Inet Address object
   * @param port 监听端口
   * @param loopbackOnly 当loopbackOnly为true时,绑定地址为127.0.0.1,只允许本地进程访问
   * 当loopbackOnly为false的时候,绑定地址为0.0.0.0,允许任何网络接口访问
   */
  explicit InetAddress(DFLK_UINT16 port = 0, bool loopbackOnly = false);

  InetAddress(StringArg ip, DFLK_UINT16 port);
  InetAddress(StringArg ip, DFLK_UINT16 port, bool ipv6);

  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}
  explicit InetAddress(const struct sockaddr_in6 &addr6) : addr6_(addr6) {}

  sa_family_t family() const { return addr_.sin_family; }
  DFLK_UINT16 port() const;
  std::string toIp() const;
  std::string toIpPort() const;

  // @brief 获取套接字地址
  const struct sockaddr* getSockAddr() const { return reinterpret_cast<const struct sockaddr*>(&addr_); }

  // @brief 设置套接字地址
  void setSockAddr(const struct sockaddr_in &addr) { addr_ = addr; }

  const struct sockaddr *getSockAddrInet() const {
    return sockaddr_cast(&addr6_);
  }
  const struct sockaddr *getSockAddrInet6() const {
    return reinterpret_cast<const struct sockaddr *>(&addr6_);
  }
  void setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

  DFLK_UINT32 ipv4NetEndian() const;
  DFLK_UINT16 portNetEndian() const { return addr_.sin_port; }

  static bool resolve(StringArg hostname, InetAddress* result);

private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}   // namespace net

}   // namespace flkeeper

#endif
