#ifndef SRPC_NET_BUFFER_H
#define SRPC_NET_BUFFER_H

#include <string>

const size_t kCapacity = 1024;

// 这是 net 模块的 应用层缓冲
// 相当于是 in-bound 或 out-bound 的 byte stream 缓冲
// not thread safe
class Buffer {
public:
    Buffer(size_t capacity = kCapacity)
        : _storage()
        , _capacity(capacity)
    { _storage.reserve(_capacity); }

    size_t readable() const { return _write_pos - _read_pos; }
    size_t writeable() const { return _capacity - _write_pos; }

    bool empty() const { return _write_pos == _read_pos; }

    // 从 buffer 的可读位置开始，偷瞄 len 个字节
    std::string peek(size_t len) const;

    // 从 buffer 的可读位置开始，删除 len 个字节
    void pop_out(size_t len);

    // 从 buffer 中读出 len 个字节
    // param len: 要读出来的字节长度
    std::string retrieve(size_t len) {
        std::string ret = peek(len);
        pop_out(len);
        return ret;
    }

    // 从 buffer 中读出全部字节
    std::string retrieve_all() { return retrieve(readable()); }

    // 剩余容量，或剩余可写入的空间
    size_t remain_capacity() const { return _capacity - readable(); }

    // 往 buffer 写数据
    // param data: 要写入的数据
    // return: 返回写入的长度
    size_t append(const std::string& data);

    // 重置 buffer
    void reset() {
        _read_pos = _write_pos = 0;
    }
private:
    // 获取可读内容的起始位置
    const char* read_begin() const { return _storage.data() + _read_pos; }

    // 扩容
    // param n: 至少需要 n 个字节的空间
    void reserve(size_t n);
private:
    std::string _storage;
    size_t _capacity{};
    size_t _write_pos{};
    size_t _read_pos{};
};

#endif
