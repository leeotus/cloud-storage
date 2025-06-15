#include "network/InetAddress.hpp"
#include "utils/log/Logging.hpp"
#include "network/Endian.hpp"

#include <netdb.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <strings.h>

namespace flkeeper {
namespace network {

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

/**
 * @brief 检查IP地址字符串是否为IPv6地址
 */
static bool isIpV6Address(const char *ip) {
  if (::strchr(ip, ':') != nullptr) {
    return true;
  }
  return false;
}

InetAddress::InetAddress(DFLK_UINT16 port, bool loopbackOnly) {
  ::bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
  addr_.sin_addr.s_addr = hostToNetwork32(ip);
  addr_.sin_port = hostToNetwork16(port);
}

InetAddress::InetAddress(StringArg ip, DFLK_UINT16 port) {
  if (isIpV6Address(ip.c_str())) {
    ::bzero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr) <= 0) {
      LOG_ERROR << "InetAddress::InetAddress invalid IPv6 addr";
    }
  } else {
    ::bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
      LOG_ERROR << "InetAddress::InetAddress invalid IPv4 addr";
    }
  }
}

InetAddress::InetAddress(StringArg ip, DFLK_UINT16 port, bool ipv6) {
  if (ipv6) {
    ::bzero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr) <= 0) {
      LOG_ERROR << "InetAddress::InetAddress invalid IPv6 addr";
    }
  } else {
    ::bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
      LOG_ERROR << "InetAddress::InetAddress invalid IPv4 addr";
    }
  }
}

std::string InetAddress::toIp() const {
  char buf[64] = "";
  if (family() == AF_INET) {
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  } else if (family() == AF_INET6) {
    ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, sizeof buf);
  }
  return buf;
}

std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  if (family() == AF_INET) {
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = ::strlen(buf);
    uint16_t port = networkToHost16(addr_.sin_port);
    snprintf(buf + end, sizeof buf - end, ":%u", port);
  } else if (family() == AF_INET6) {
    buf[0] = '[';
    ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf + 1, sizeof buf - 1);
    size_t end = ::strlen(buf);
    uint16_t port = networkToHost16(addr6_.sin6_port);
    snprintf(buf + end, sizeof buf - end, "]:%u", port);
  }
  return buf;
}

uint16_t InetAddress::port() const {
  return networkToHost16(portNetEndian());
}

uint32_t InetAddress::ipv4NetEndian() const {
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

bool InetAddress::resolve(StringArg hostname, InetAddress *result) {
  assert(result != nullptr);
  struct hostent *ent = nullptr;
  struct hostent hent;
  char buf[8192];
  int herrno = 0;
  ::bzero(&hent, sizeof hent);
  int ret =
      gethostbyname_r(hostname.c_str(), &hent, buf, sizeof buf, &ent, &herrno);
  if (ret == 0 && ent != nullptr) {
    assert(ent->h_addrtype == AF_INET);
    result->addr_.sin_addr = *reinterpret_cast<struct in_addr *>(ent->h_addr);
    return true;
  } else {
    if (ret) {
      LOG_ERROR << "InetAddress::resolve";
    }
    return false;
  }
}

} // namespace network
} // namespace flkeeper
