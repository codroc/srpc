#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <stdint.h>
#include <memory>
#include <string>

// 模仿 google protobuf 的序列化规则
// Tag [Length] Value
// TLV: string
// TV: varint, fixed32, fiexd64
//
class Serialize {
public:
    using ptr = std::shared_ptr<Serialize>;
    enum type {
        SERIALIZER,
        DESERIALIZER
    };
    // 对应于 serializer
    Serialize(type t, size_t base_size = 4096);
    // 对应于 deserializer
    Serialize(type t, const std::string& bytearray);

    ~Serialize();

    // deserializer
    void reset();
    void reset(const std::string& bytearray);

    // write
    // 序列化从 p 开始的 4 个字节到 
    // 序列化失败返回 -1，成功返回 0
    int writeBit32(void* p);
    int writeBit64(void* p);
    void writeFixed32(uint32_t value);
    void writeFixed64(uint64_t value);
    void writeSFixed32(int32_t value);
    void writeSFixed64(int64_t value);

    void writeVarInt8(int8_t value);
    void writeVarUint8(uint8_t value);
    void writeVarInt16(int16_t value);
    void writeVarUint16(uint16_t value);
    void writeVarInt32(int32_t value);
    void writeVarUint32(uint32_t value);
    void writeVarInt64(int64_t value);
    void writeVarUint64(uint64_t value);

    void writeFloat(float value);
    void writeDouble(double value);

    void writeString(const std::string& str);

    // read
    int readBit32(void* p);
    int readBit64(void* p);
    uint32_t readFixed32();
    uint64_t readFixed64();
    int32_t readSFixed32();
    int64_t readSFixed64();

    int8_t readVarInt8();
    uint8_t readVarUint8();
    int16_t readVarInt16();
    uint16_t readVarUint16();
    int32_t readVarInt32();
    uint32_t readVarUint32();
    int64_t readVarInt64();
    uint64_t readVarUint64();

    float readFloat();
    double readDouble();

    std::string readString();

    // 写到 Node 中去
    void write(const void* buf, size_t len);

    // 从 Node 中读出来
    void read(void* buf, size_t len);

    // 把 Node 中的所有信息放到 string 中返回
    std::string toString();

    // serialize to file
    // 前提是 Serialize 对象里已经保存了 序列化过的字符串了
    static bool toFile(const std::string& filepath, const std::string& str);
    // 从文件获取序列化的字符串
    // 前提假设是 file 中存在合规范的序列化字符串
    static std::string fromFile(const std::string& filepath);
private:
    uint16_t encodeZigZag16(int16_t value);
    uint32_t encodeZigZag32(int32_t value);
    uint64_t encodeZigZag64(int64_t value);
    int16_t decodeZigZag16(uint16_t value);
    int32_t decodeZigZag32(uint32_t value);
    int64_t decodeZigZag64(uint64_t value);
private:
    struct Node {
        Node(size_t base_size);
        ~Node();

        // 对于拷贝函数来说，因为这里的拷贝涉及深浅拷贝，也不知道后面用不用得到，所以先 delete 掉，后面用到的时候再说
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        char *str{nullptr};
        size_t base_size{0};
        Node* next{nullptr};
    };

    type _type;
    size_t _base_size{4096}; // 默认一个 Node 占用一页
    Node* _head{nullptr}; // Node 链表头
    Node* _tail{nullptr}; // Node 链表尾
    char*  _cur{nullptr};
    uint32_t _capacity{0}; // 序列化容器 当前容量
    uint32_t _size{0}; // 已经写了多少个字节

    // deserializer
    char* _buf{nullptr};
    size_t _len{0};
    uint32_t _readed{0}; // 对于 deserializer 来说已经读了的字节数
};

#endif
