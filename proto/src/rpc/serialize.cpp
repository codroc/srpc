#include "rpc/serialize.h"
#include <assert.h>
#include <byteswap.h>
#include <fstream>

#include "flog.h"

namespace srpc {
namespace rpc {

bool isLittleEndian();
static bool g_isLittleEndian = isLittleEndian();

bool isLittleEndian() {
    unsigned short a = 0x1234;
    char* p = (char*) &a;
    if (*((uint8_t*)&p[0]) == 0x12) return false;
    return true;
}

Serialize::Node::Node(size_t base_size)
{
    this->base_size = base_size;
    str = (char*) ::malloc(base_size);
    if (str == nullptr) {
        LOG_ERROR << "OOM!\n";
        exit(1);
    }
}

Serialize::Node::~Node() {
    if (str) {
        ::free(str);
        str = nullptr;
    }
}

Serialize::Serialize(Serialize::type t, size_t base_size)
    : _type(t),
      _base_size(base_size),
      _head(new Node(_base_size)),
      _tail(_head),
      _capacity(_base_size)
{
    _cur = _head->str; // 能否保证 Node 以及构造完成？
}

Serialize::Serialize(Serialize::type t, const std::string& bytearray) 
    : _type(t)
{
    assert(t == DESERIALIZER && bytearray.size() > 0);
    _buf = (char*) ::malloc(bytearray.size());
    if (_buf == nullptr) {
        LOG_ERROR << "OOM\n";
        exit(1);
    }
    _len = bytearray.size();
    ::memmove(_buf, &bytearray[0], _len);
}

Serialize::~Serialize() {
    // 释放完 Node 链表
    while(_head) {
        Node* tmp = _head;
        _head = _head->next;
        delete tmp;
    }
    // 释放 反序列化时 malloc 出来的 buf
    if (_buf) {
        ::free(_buf);
        _buf = nullptr;
    }
}

void Serialize::reset() {
    assert(_type == DESERIALIZER);
    if (_buf) {
        ::free(_buf);
        _buf = nullptr;
        _len = 0;
        _readed = 0;
    }
}

void Serialize::reset(const std::string& bytearray) {
    reset();
    _buf = (char*) ::malloc(bytearray.size());
    if (_buf == nullptr) {
        LOG_ERROR << "OOM\n";
        exit(1);
    }
    _len = bytearray.size();
    ::memmove(_buf, &bytearray[0], _len);
}

// 写到 Node 中去
void Serialize::write(const void* buf, size_t len) {
    assert(_type == SERIALIZER);
    if (_capacity > len) {
        ::memmove(_cur, buf, len);
        _cur += len;
        _capacity -= len;
        _size += len;
    } else {
        size_t remain = len - _capacity;
        const char* p = (const char*) buf;
        ::memmove(_cur, p, _capacity);
        p += _capacity;
        _size += _capacity;
        _capacity = 0;

        while (remain > _base_size) {
            _tail->next = new Node(_base_size);
            _tail = _tail->next;
            
            ::memmove(_tail->str, p, _base_size);
            p += _base_size;
            _size += _base_size;
            remain -= _base_size;
        }

        _tail->next = new Node(_base_size);
        _tail = _tail->next;
             
        ::memmove(_tail->str, p, remain);
        _cur += remain;
        _size += remain;
        _capacity = _base_size - remain;
    }
}

// 从 Node 中读出来
void Serialize::read(void* buf, size_t len) {
    assert(_type == DESERIALIZER);
    assert(len + _readed <= _len);
    // 对于反序列化的一端，从 _buf 起读出 len 个字节放到 buf 中去
    char* p = _buf + _readed;
    ::memmove(buf, p, len);
    _readed += len;
}

uint16_t Serialize::encodeZigZag16(int16_t value) {
    return value < 0 ? ((uint16_t)(-value) * 2 - 1) : (2 * (uint16_t)value);
}

uint32_t Serialize::encodeZigZag32(int32_t value) {
    return value < 0 ? ((uint32_t)(-value) * 2 - 1) : (2 * (uint32_t)value);
}

uint64_t Serialize::encodeZigZag64(int64_t value) {
    return value < 0 ? ((uint64_t)(-value) * 2 - 1) : (2 * (uint64_t)value);
}

int16_t Serialize::decodeZigZag16(uint16_t value) {
    return (value >> 1) ^ -(value & 1);
}

int32_t Serialize::decodeZigZag32(uint32_t value) {
    return (value >> 1) ^ -(value & 1);
}

int64_t Serialize::decodeZigZag64(uint64_t value) {
    return (value >> 1) ^ -(value & 1);
}

// write
// 序列化从 p 开始的 4 个字节到 
// 序列化失败返回 -1，成功返回 0
// fixed32 可以是 int32 uint32 float
int Serialize::writeBit32(void* p) {
    uint32_t v;
    ::memcpy(&v, p, sizeof v);
    if (g_isLittleEndian) {
        v = ::bswap_32(v);
    }
    write(&v, sizeof v);
    return 0;
}

int Serialize::writeBit64(void* p) {
    uint64_t v;
    ::memcpy(&v, p, sizeof v);
    if (g_isLittleEndian) {
        v = ::bswap_64(v);
    }
    write(&v, sizeof v);
    return 0;
}

void Serialize::writeFixed32(uint32_t value) {
    writeBit32(&value);
}

void Serialize::writeFixed64(uint64_t value) {
    writeBit64(&value);
}

void Serialize::writeSFixed32(int32_t value) {
    writeFixed32(encodeZigZag32(value));
}

void Serialize::writeSFixed64(int64_t value) {
    writeFixed64(encodeZigZag64(value));
}

void Serialize::writeVarInt8(int8_t value) {
    // 这个不需要压缩，也不需要转大端序，直接字节拷贝就行了
    write(&value, sizeof value);
}

void Serialize::writeVarUint8(uint8_t value) {
    write(&value, sizeof value);
}

#define XX(n) \
    char tmp[n]; \
    int i = 0; \
    while (value >= 0x80) { \
        tmp[i++] = static_cast<uint8_t>(value) | 0x80; \
        value >>= 7; \
    } \
    tmp[i++] = static_cast<uint8_t>(value); \
    write(tmp, i)

void Serialize::writeVarInt16(int16_t value) {
    writeVarUint16(encodeZigZag16(value));
}

void Serialize::writeVarUint16(uint16_t value) {
    XX(3);
}

void Serialize::writeVarInt32(int32_t value) {
    writeVarUint32(encodeZigZag32(value));
}

void Serialize::writeVarUint32(uint32_t value) {
    XX(5);
}

void Serialize::writeVarInt64(int64_t value) {
    writeVarUint64(encodeZigZag64(value));
}

void Serialize::writeVarUint64(uint64_t value) {
    XX(10);
}
#undef XX

void Serialize::writeFloat(float value) {
    writeBit32(&value);
}

void Serialize::writeDouble(double value) {
    writeBit64(&value);
}

void Serialize::writeString(const std::string& str) {
    // LV: Length Value
    // 其中 Length 也是 varint 编码
    std::string tmp{str};
    uint32_t len = tmp.size(); // 默认最长的 string 是 4G
    writeVarUint32(len);
    if (len == 0)   return;
    write(&tmp[0], len);
}

// read
int Serialize::readBit32(void* p) {
    read(p, sizeof(uint32_t));
    if (isLittleEndian) {
        *(uint32_t*)p = bswap_32(*(uint32_t*)p);
    }
    return 0;
}

int Serialize::readBit64(void* p) {
    read(p, sizeof(uint64_t));
    if (isLittleEndian) {
        *(uint64_t*)p = bswap_64(*(uint64_t*)p);
    }
    return 0;
}

uint32_t Serialize::readFixed32() {
    uint32_t v;
    readBit32(&v);
    return v;
}

uint64_t Serialize::readFixed64() {
    uint64_t v;
    readBit64(&v);
    return v;
}

int32_t Serialize::readSFixed32() {
    return decodeZigZag32(readFixed32());
}

int64_t Serialize::readSFixed64() {
    return decodeZigZag64(readFixed64());
}


// TODO: 存在 bug，当 int8 最高位为 1 时，还是需要用 两个字节来保存
int8_t Serialize::readVarInt8() {
    int8_t v;
    read(&v, sizeof v);
    return v;
}

uint8_t Serialize::readVarUint8() {
    uint8_t v;
    read(&v, sizeof v);
    return v;
}

#define XX(n) \
    uint##n##_t ret = 0; \
    for (int i = 0; i < n; i+=7) { \
        uint8_t b; \
        read(&b, 1); \
        if (b < 0x80) { \
            ret |= static_cast<uint##n##_t>(b) << i; \
            break; \
        } \
        ret |= static_cast<uint##n##_t>(b & 0x7f) << i; \
    } \
    return ret

int16_t Serialize::readVarInt16() {
    return decodeZigZag16(readVarUint16());
}

uint16_t Serialize::readVarUint16() {
    XX(16);
}

int32_t Serialize::readVarInt32() {
    return decodeZigZag32(readVarUint32());
}

uint32_t Serialize::readVarUint32() {
    XX(32);
}

int64_t Serialize::readVarInt64() {
    return decodeZigZag64(readVarUint64());
}

uint64_t Serialize::readVarUint64() {
    XX(64);
}

#undef XX

float Serialize::readFloat() {
    float f;
    readBit32(&f);
    return f;
}

double Serialize::readDouble() {
    double d;
    readBit64(&d);
    return d;
}

std::string Serialize::readString() {
    uint32_t len = readVarUint32();
    if (len == 0)   return {};
    std::string ret;
    ret.resize(len);
    read(&ret[0], len);
    return ret;
}

std::string Serialize::toString() {
    std::string ret;
    Node* tmp = _head;
    uint32_t s = _size;
    while (tmp && s > _base_size) {
        ret.append(tmp->str, _base_size);
        s -= _base_size;
        tmp = tmp->next;
    }
    ret.append(tmp->str, s);
    return ret;
}

bool Serialize::toFile(const std::string& filepath, const std::string& str) {
    std::ofstream of(filepath);
    if (of.fail()) {
        LOG_ERROR << "can not open file: " << filepath << "\n";
        return false;
    }
    of << str;
    of.close();
    return true;
}

std::string Serialize::fromFile(const std::string& filepath) {
    std::ifstream infile(filepath);
    if (infile.fail()) {
        LOG_ERROR << "can not open file: " << filepath << "\n";
        return {};
    }
    std::string ret;
    infile >> ret;
    infile.close();
    return ret;
}

}
}
