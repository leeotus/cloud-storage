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
  explicit InetAddress(DFLK_UINT16 port = 0, bool loopbackOnly = false);
private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}   // namespace net

}   // namespace flkeeper

#endif
