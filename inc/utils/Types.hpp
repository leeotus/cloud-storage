#ifndef __CLOUD_STORAGE_TYPES_HPP__
#define __CLOUD_STORAGE_TYPES_HPP__

#include <string>

namespace flkeeper {

using DFLK_INT8 = ::int8_t;
using DFLK_INT16 = ::int16_t;
using DFLK_INT32 = ::int32_t;
using DFLK_INT64 = ::int64_t;

using DFLK_UINT8 = ::uint8_t;
using DFLK_UINT16 = ::uint16_t;
using DFLK_UINT32 = ::uint32_t;
using DFLK_UINT64 = ::uint64_t;

using DFLK_FP = double;

// 去除类型引用
template<typename T>
inline T implicit_cast(typename std::remove_reference<T>::type &x) {
  return x;
}

template<typename T>
inline T implicit_cast(const typename std::remove_reference<T>::type &x) {
  return x;
}

} // namespace flkeeper

#endif
