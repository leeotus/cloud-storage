#ifndef __CLOUD_STORAGE_ENDIAN_HPP__
#define __CLOUD_STORAGE_ENDIAN_HPP__

#include "utils/Types.hpp"
#include <cstdint>
#include <endian.h>

namespace flkeeper {
namespace network {
// 主机字节序转网络字节序
inline DFLK_UINT64 hostToNetwork64(DFLK_UINT64 host64) {
    return htobe64(host64);
}

inline DFLK_UINT32 hostToNetwork32(DFLK_UINT32 host32) {
    return htobe32(host32);
}

inline DFLK_UINT16 hostToNetwork16(DFLK_UINT16 host16) {
    return htobe16(host16);
}

// 网络字节序转主机字节序
inline DFLK_UINT64 networkToHost64(DFLK_UINT64 net64) {
    return be64toh(net64);
}

inline DFLK_UINT32 networkToHost32(DFLK_UINT32 net32) {
    return be32toh(net32);
}

inline DFLK_UINT16 networkToHost16(DFLK_UINT16 net16) {
    return be16toh(net16);
}
}   // namespace network
}   // namespace flkeeper

#endif
