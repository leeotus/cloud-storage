#ifndef __CLOUD_STORAGE_CALLBACK_HPP__
#define __CLOUD_STORAGE_CALLBACK_HPP__

#include <functional>
#include <memory>
#include "utils/date/TimeStamp.hpp"
#include "Buffer.hpp"

namespace flkeeper {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

template<typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr) {
    return ptr.get();
}

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr) {
    return ptr.get();
}

namespace network {

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// All client visible callbacks go here.
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;
typedef std::function<void (const TcpConnectionPtr&,
                            Buffer*,
                            date::TimeStamp)> MessageCallback;

}   // namespace network
}   // namespace flkeeper

#endif
