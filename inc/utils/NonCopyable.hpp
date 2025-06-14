#ifndef __CLOUD_STORAGE_NON_COPYABLE_HPP__
#define __CLOUD_STORAGE_NON_COPYABLE_HPP__

namespace flkeeper {

// 用于标记不可拷贝的类
class NonCopyable{
public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

}

#endif
