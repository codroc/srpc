#include "buffer.h"
#include <string.h>
#include <arpa/inet.h>

std::string Buffer::peek(size_t len) const {
    if (len > readable()) len = readable();
    return {read_begin(), len};
}

void Buffer::pop_out(size_t len) {
    if (len > readable()) len = readable();
    _read_pos += len;
    if (_read_pos == _write_pos) reset();
}

size_t Buffer::append(const char* p, size_t len) {
    return append(std::string(p, len));
}

size_t Buffer::append(const std::string& data) {
    size_t n = data.size();
    if (n <= remain_capacity()) {
        if (n <= writeable()) {
        } else {
            // 简单的 memmove 就可以了，不需要额外分配空间
            ::memmove(_storage.data(), read_begin(), readable());
            _read_pos = 0;
            _write_pos = readable();
        }
    } else {
        // 扩容
        reserve(n + readable());
    }

    // 写入内容
    _storage.append(data);
    _write_pos += n;
    return n;
}

void Buffer::reserve(size_t n) {
    std::string new_storage = retrieve_all();
    new_storage.reserve(n);
    _storage.swap(new_storage);
    _read_pos = 0;
    _write_pos = _storage.size();
    _capacity = _storage.capacity();
}

int32_t Buffer::peek_int32() const {
    // Buffer 中的内容默认是网络字节序
    const uint32_t *p = reinterpret_cast<const uint32_t*>(read_begin());
    return ::ntohl(*p);
}

void Buffer::append_int32(int32_t val) {
    uint32_t v = ::htonl(static_cast<uint32_t>(val));
    append(reinterpret_cast<char*>(&v), sizeof v);
}
