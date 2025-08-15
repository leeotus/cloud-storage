#ifndef __CLOUD_STORAGE_LINKED_BUFFER_HPP__
#define __CLOUD_STORAGE_LINKED_BUFFER_HPP__

#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <list>
#include <sys/uio.h>
#include <errno.h>
#include <algorithm>

// 单块内存默认大小（可根据场景调整）
#define LINKED_BLOCK_SIZE 4096
// 每块内存前置空间大小（用于头部插入）
#define LINKED_PREPEND_SIZE 8

namespace flkeeper {
namespace network {

/**
 * @brief 链式缓冲区类（多块离散内存 + 链表管理）
 * 缓冲区由多个内存块组成，每个块内部划分为三个区域：
 * +-----------------+--------------------+-----------------+
 * |   prependable   |      readable      |     writeable   |
 * +-----------------+--------------------+-----------------+
 * 功能与原Buffer一致，但通过链表实现离散内存管理
 */
class LinkedBuffer {
private:
    // 单个内存块结构
    struct Block {
        char* data;               // 块内存起始地址
        size_t prependable;       // 前置区域大小（已使用）
        size_t readable;          // 可读区域大小（有效数据）
        size_t writable;          // 可写区域大小（空闲空间）
        size_t capacity;          // 总容量（prependable + readable + writable）

        Block(size_t cap = LINKED_BLOCK_SIZE)
            : capacity(cap),
              prependable(LINKED_PREPEND_SIZE),  // 预留前置空间
              readable(0),
              writable(cap - LINKED_PREPEND_SIZE) {  // 剩余空间为可写
            data = new char[capacity];
        }

        ~Block() {
            delete[] data;
        }

        // 可读数据起始地址
        const char* peek() const {
            return data + prependable;
        }

        // 可写空间起始地址
        char* beginWrite() {
            return data + prependable + readable;
        }

        // 收缩块空间（仅保留必要容量）
        void shrink() {
            if (prependable + readable + writable != capacity) return;
            size_t new_cap = prependable + readable + 1;  // 保留1字节可写
            char* new_data = new char[new_cap];
            memcpy(new_data + prependable, peek(), readable);  // 拷贝有效数据
            delete[] data;
            data = new_data;
            capacity = new_cap;
            writable = capacity - prependable - readable;
        }
    };

public:
    LinkedBuffer() : total_readable_(0) {}

    ~LinkedBuffer() {
        clear();
    }

    // 禁止拷贝（简化线程安全处理）
    LinkedBuffer(const LinkedBuffer&) = delete;
    LinkedBuffer& operator=(const LinkedBuffer&) = delete;

    // 可读字节总数
    size_t readableBytes() const {
        return total_readable_;
    }

    // 可写字节总数（所有块可写空间之和）
    size_t writableBytes() const {
        size_t total = 0;
        for (const auto& block : blocks_) {
            total += block->writable;
        }
        return total;
    }

    // 所有块前置空间总和（用于头部插入）
    size_t prependableBytes() const {
        size_t total = 0;
        for (const auto& block : blocks_) {
            total += block->prependable;
        }
        return total;
    }

    // 查找CRLF分隔符（跨块处理）
    const char* findCRLF() const {
        const char* crlf = nullptr;
        size_t remaining = total_readable_;
        for (const auto& block : blocks_) {
            if (block->readable == 0) continue;
            const char* start = block->peek();
            const char* end = start + block->readable;
            // 在当前块中查找CRLF
            crlf = std::search(start, end, kCRLF, kCRLF + 2);
            if (crlf != end) {
                return crlf;  // 找到则返回
            }
            remaining -= block->readable;
            if (remaining < 2) break;  // 剩余数据不足2字节，不可能有CRLF
        }
        return nullptr;
    }

    // 从指定位置开始查找CRLF
    const char* findCRLF(const char* start) const {
        // 先定位start所在的块
        for (const auto& block : blocks_) {
            const char* block_start = block->peek();
            const char* block_end = block_start + block->readable;
            if (start >= block_start && start < block_end) {
                // 在当前块内查找
                const char* crlf = std::search(start, block_end, kCRLF, kCRLF + 2);
                return crlf == block_end ? nullptr : crlf;
            }
        }
        return nullptr;
    }

    // 查找换行符（跨块处理）
    const char* findEOL() const {
        for (const auto& block : blocks_) {
            if (block->readable == 0) continue;
            const void* eol = memchr(block->peek(), '\n', block->readable);
            if (eol) return static_cast<const char*>(eol);
        }
        return nullptr;
    }

    // 从指定位置查找换行符
    const char* findEOL(const char* start) const {
        for (const auto& block : blocks_) {
            const char* block_start = block->peek();
            const char* block_end = block_start + block->readable;
            if (start >= block_start && start < block_end) {
                size_t len = block_end - start;
                const void* eol = memchr(start, '\n', len);
                return static_cast<const char*>(eol);
            }
        }
        return nullptr;
    }

    // 读取len个字节（移动读指针）
    void retrieve(size_t len) {
        assert(len <= total_readable_);
        size_t remaining = len;
        // 遍历块，消耗可读数据
        while (remaining > 0 && !blocks_.empty()) {
            Block* front = blocks_.front();
            if (front->readable <= remaining) {
                remaining -= front->readable;
                total_readable_ -= front->readable;
                // 块数据已读完，回收块
                delete front;
                blocks_.pop_front();
            } else {
                front->readable -= remaining;
                front->prependable += remaining;  // 前置空间增加（读指针后移）
                total_readable_ -= remaining;
                remaining = 0;
            }
        }
    }

    // 读取到指定位置
    void retrieveUntil(const char* end) {
        // 先定位end所在的块
        for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
            Block* block = *it;
            const char* block_start = block->peek();
            const char* block_end = block_start + block->readable;
            if (end >= block_start && end <= block_end) {
                size_t len = end - block_start;
                retrieve(len);  // 读取到end位置
                return;
            } else if (end > block_end) {
                // 跳过当前块（全部读取）
                total_readable_ -= block->readable;
                delete block;
                blocks_.erase(it);
            }
        }
    }

    // 读取所有数据
    void retrieveAll() {
        clear();
        total_readable_ = 0;
    }

    // 读取所有数据并转为string
    std::string retrieveAllAsString() {
        return retrieveAsString(total_readable_);
    }

    // 读取len个字节并转为string
    std::string retrieveAsString(size_t len) {
        assert(len <= total_readable_);
        std::string result;
        result.reserve(len);
        size_t remaining = len;
        // 遍历块，拼接数据
        while (remaining > 0 && !blocks_.empty()) {
            Block* front = blocks_.front();
            size_t copy_len = std::min(remaining, front->readable);
            result.append(front->peek(), copy_len);
            remaining -= copy_len;
            // 更新块状态
            front->readable -= copy_len;
            front->prependable += copy_len;
            total_readable_ -= copy_len;
            // 若块已空，回收
            if (front->readable == 0) {
                delete front;
                blocks_.pop_front();
            }
        }
        return result;
    }

    // 追加数据（自动申请新块）
    void append(const char* data, size_t len) {
        if (len == 0) return;
        size_t remaining = len;
        const char* cur = data;
        // 优先填充已有块的可写空间
        while (remaining > 0) {
            if (blocks_.empty() || blocks_.back()->writable == 0) {
                // 无块或最后一块已满，新建块
                size_t new_block_size = std::max<size_t>(LINKED_BLOCK_SIZE, remaining);
                blocks_.push_back(new Block(new_block_size));
            }
            Block* back = blocks_.back();
            size_t write_len = std::min(remaining, back->writable);
            memcpy(back->beginWrite(), cur, write_len);
            back->readable += write_len;
            back->writable -= write_len;
            total_readable_ += write_len;
            cur += write_len;
            remaining -= write_len;
        }
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    // 头部插入数据（仅使用第一个块的前置空间）
    void prepend(const void* data, size_t len) {
        if (len == 0) return;
        assert(!blocks_.empty() && blocks_.front()->prependable >= len);
        Block* front = blocks_.front();
        front->prependable -= len;
        const char* d = static_cast<const char*>(data);
        memcpy(front->data + front->prependable, d, len);
    }

    // 收缩空间（释放空块，压缩非空块）
    void shrink(size_t reserve) {
        // 释放空块
        auto it = blocks_.begin();
        while (it != blocks_.end()) {
            if ((*it)->readable == 0) {
                delete *it;
                it = blocks_.erase(it);
            } else {
                (*it)->shrink();  // 压缩非空块
                ++it;
            }
        }
        // 若需预留空间，确保最后一块有足够可写空间
        if (writableBytes() < reserve && !blocks_.empty()) {
            size_t need = reserve - writableBytes();
            blocks_.back()->writable += need;
            blocks_.back()->capacity += need;
            char* new_data = new char[blocks_.back()->capacity];
            memcpy(new_data + blocks_.back()->prependable,
                   blocks_.back()->peek(),
                   blocks_.back()->readable);
            delete[] blocks_.back()->data;
            blocks_.back()->data = new_data;
        }
    }

    // 从文件描述符读取数据（使用readv分散读）
    ssize_t readFd(int fd, int* savedErrno) {
        std::vector<struct iovec> iovs;
        // 收集已有块的可写空间
        for (auto& block : blocks_) {
            if (block->writable > 0) {
                iovs.push_back({block->beginWrite(), block->writable});
            }
        }
        // 准备新块（防止现有空间不足）
        Block* new_block = new Block();
        blocks_.push_back(new_block);
        iovs.push_back({new_block->beginWrite(), new_block->writable});

        // 执行分散读
        const ssize_t n = ::readv(fd, iovs.data(), iovs.size());
        if (n < 0) {
            *savedErrno = errno;
            // 移除未使用的新块
            blocks_.pop_back();
            delete new_block;
        } else if (static_cast<size_t>(n) > 0) {
            size_t bytes_read = static_cast<size_t>(n);
            total_readable_ += bytes_read;
            // 更新块的可写/可读状态
            auto it = blocks_.begin();
            for (size_t i = 0; i < iovs.size() && bytes_read > 0; ++i) {
                std::advance(it, i);  // 将迭代器向前移动i个位置
                Block* block = (i < blocks_.size() - 1) ? *it : new_block;
                // Block* block = (i < blocks_.size() - 1) ? blocks_[i] : new_block;
                size_t write_len = std::min(bytes_read, block->writable);
                block->readable += write_len;
                block->writable -= write_len;
                bytes_read -= write_len;
            }
            // 若新块未使用，回收
            if (new_block->writable == LINKED_BLOCK_SIZE - LINKED_PREPEND_SIZE) {
                blocks_.pop_back();
                delete new_block;
            }
        } else {
            // n=0（对方关闭连接），回收新块
            blocks_.pop_back();
            delete new_block;
        }
        return n;
    }

    // 写入文件描述符（使用writev集中写）
    ssize_t writeFd(int fd, int* savedErrno) {
        if (total_readable_ == 0) return 0;
        std::vector<struct iovec> iovs;
        // 收集所有块的可读数据
        for (const auto& block : blocks_) {
            if (block->readable > 0) {
                iovs.push_back({const_cast<char*>(block->peek()), block->readable});
            }
        }
        // 执行集中写
        const ssize_t n = ::writev(fd, iovs.data(), iovs.size());
        if (n < 0) {
            *savedErrno = errno;
        } else if (static_cast<size_t>(n) > 0) {
            retrieve(static_cast<size_t>(n));  // 读取已写入的数据
        }
        return n;
    }

private:
    // 清空所有块
    void clear() {
        for (auto& block : blocks_) {
            delete block;
        }
        blocks_.clear();
    }

    std::list<Block*> blocks_;  // 块链表（管理所有内存块）
    size_t total_readable_;     // 总可读字节数（缓存，避免每次遍历计算）
    static const char kCRLF[];  // CRLF分隔符
};

const char LinkedBuffer::kCRLF[] = "\r\n";

} // namespace network
} // namespace flkeeper

#endif
