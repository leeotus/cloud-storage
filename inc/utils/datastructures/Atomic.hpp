#ifndef __CLOUD_STORAGE_ATOMIC_HPP__
#define __CLOUD_STORAGE_ATOMIC_HPP__

#include "utils/NonCopyable.hpp"
#include "utils/Types.hpp"

namespace flkeeper {

template<typename T>
class AtomicIntegerT : NonCopyable
{
public:
    AtomicIntegerT()
        : value_(0)
    {
    }

    // 禁止隐式类型转换
    explicit AtomicIntegerT(T x)
        : value_(x)
    {
    }

    // 原子性地读取value_的值
    T get()
    {
        return __sync_val_compare_and_swap(&value_, 0, 0);
    }

    // 原子性地将value_设置为newValue，并返回之前的值
    T getAndSet(T newValue)
    {
        return __sync_lock_test_and_set(&value_, newValue);
    }

    // 原子性地将value_加上x，并返回之前的值
    T getAndAdd(T x)
    {
        return __sync_fetch_and_add(&value_, x);
    }

    // 原子性地将value_加上x，并返回新的值
    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
    }

    // 原子性地将value_加1，并返回之前的值
    T incrementAndGet()
    {
        return addAndGet(1);
    }

    // 原子性地将value_减1，并返回之前的值
    T decrementAndGet()
    {
        return addAndGet(-1);
    }

    // 原子性地将value_加上x
    void add(T x)
    {
        getAndAdd(x);
    }

    // 原子性地将value_加1
    void increment()
    {
        incrementAndGet();
    }

    // 原子性地将value_减1
    void decrement()
    {
        decrementAndGet();
    }

private:
    volatile T value_;
};

typedef AtomicIntegerT<DFLK_INT64> AtomicInt32;
typedef AtomicIntegerT<DFLK_INT64> AtomicInt64;

}   // namespace flkeeper

#endif
