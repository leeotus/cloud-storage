#ifndef __CLD_STORAGE_COPYABLE_HPP__
#define __CLD_STORAGE_COPYABLE_HPP__

namespace flkeeper {

// 从语义来标记某个类是可拷贝的
class Copyable {
protected:
  Copyable() = default;
  ~Copyable() = default;
};

}    // namespace flkeeper

#endif
