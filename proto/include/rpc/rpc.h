// RPC 协议格式
#ifndef SRPC_PROTO_RPC_H
#define SRPC_PROTO_RPC_H

#include <string>

#define OPTIONSIZE 20

struct RPCHeader { // 总共 8 个字节
    uint16_t magic;
    uint8_t type : 1; // request/response
    uint8_t opt : 1; // 最多只能携带 20 字节的额外数据
    uint8_t serialization : 5;
    uint8_t : 1;
    uint8_t status;
    uint16_t data_length;
    uint16_t checksum;
};

struct RPCOption { // 最多只能存 20 个字节
    char* data;
    uint8_t len;
};

class RPC {
public:
    enum SerializationType {
        Local,
        ProtoBuf,
        Json,
    };
    enum Status {
        Ok,
        Error,
        Timeout,
    };
    enum Type {
        RPCRequest,
        RPCResponse,
    };
    RPC(Type type, SerializationType se_type);

    void set_serialization_type(SerializationType type);
    SerializationType set_serialization_type() const;

    bool has_opt() const { return _header.opt == 1; }
    std::string get_opt() const { return {_opt.data, _opt.len}; }

    void set_status(Status status);
    Status get_status() const;

    void set_option(char *opt, uint8_t len);
    void set_option(const std::string& opt);

    std::string to_string() const;
    RPC from_string(const std::string& str);
private:
    RPCHeader _header;
    RPCOption _opt;

    std::string _bytes; // service name, method name, args/reply
};

#endif
