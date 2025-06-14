#include "utils/FKException.hpp"
#include <execinfo.h>

namespace flkeeper {

FKException::FKException(char *what) : message_(what){
  trace();
}

FKException::FKException(const char *what) : message_(what) {
  trace();
}

void FKException::trace() {
  constexpr int len = 200;
  void *buffer[len];

  // 捕获当前线程的堆栈信息,保存在buffer数组中. 返回实际捕获到的堆栈帧数量
  int nptrs = ::backtrace(buffer, len);

  // 获取堆栈符号信息
  char **strings = ::backtrace_symbols(buffer, nptrs);

  if (strings) {
    for (int i = 0; i < nptrs; ++i) {
      // TODO: 可以使用 abi::__cxa_demangle 来对函数名进行反混淆
      stack_.append(strings[i]);
      stack_.push_back('\n');
    }
    free(strings);
  }
}

} // namespace flkeeper
