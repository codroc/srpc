// RPC 协议格式
#ifndef SRPC_PROTO_RPC_H
#define SRPC_PROTO_RPC_H

#include "serialize.h"
#include <string>

#define RPC_OPTIONSIZE 20

namespace srpc {
namespace rpc {

struct RPCHeader { // 总共 8 个字节
    RPCHeader();
    uint16_t get_magic_number();

    uint16_t magic;
    uint8_t type : 2; // request/response
    uint8_t opt : 1; // 最多只能携带 20 字节的额外数据
    uint8_t serialization : 5;
    uint8_t status;
    uint16_t data_length;
    uint16_t checksum;
};

struct RPCOption { // 最多只能存 20 个字节
    RPCOption()
        : data(0)
    {}
    ~RPCOption() {
        if (data) {
            ::free(data);
            data = 0;
        }
    }
    void reset() {
        this->~RPCOption();
    }
    void set(const char* opt, uint8_t length);
    void setNewOpt(const char* opt, uint8_t length) {
        reset();
        set(opt, length);
    }
    char* data;
};

class RPCPackage {
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
        HeartBeat,
        RPCRequest,
        RPCResponse,
    };
    RPCPackage();
    RPCPackage(Type type, SerializationType se_type);

    void set_serialization_type(SerializationType type) { _header.serialization = type; }
    SerializationType get_serialization_type() const { return static_cast<SerializationType>(_header.serialization); }

    bool has_opt() const { return _header.opt; }
    std::string get_opt() const { return {_opt.data, RPC_OPTIONSIZE}; }

    void set_status(Status status) { _header.status = status; }
    Status get_status() const { return static_cast<Status>(_header.status); }

    void set_option(const char *opt, uint8_t len);
    void set_option(const std::string& opt) { set_option(opt.data(), opt.size()); }

    void set_header(RPCHeader header) { _header = header; }
    RPCHeader get_header() const { return _header; }

    void set_request() { _header.type = RPCRequest; }
    void set_response() { _header.type = RPCResponse; }
    void set_heartbeat() { _header.type = HeartBeat; }

    void set_body(const std::string& bytes) { set_body(bytes.data(), bytes.size()); }
    void set_body(const char* data, int len);
    std::string get_body() const { return _bytes; }

    void set_arg_or_reply();
    std::string get_arg_or_reply() const { return _arg_or_reply; }

    void set_checksum() {}
    uint16_t get_checksum() const { return _header.checksum; }

    std::string to_string() const;
    static RPCPackage from_string(const std::string& str);
private:
    RPCHeader _header;
    RPCOption _opt;

    std::string _bytes; // service name, method name, args/reply
    std::string _arg_or_reply;
};

struct RPCMethod {
    RPCMethod()
        : valid(false), service_name(), method_name(), args_type(), reply_type() {}
    RPCMethod(const std::string& s, const std::string& m, const std::string& args, const std::string& reply)
        : valid(true), service_name(s), method_name(m), args_type(args), reply_type(reply) {}

    bool valid;
    std::string service_name;
    std::string method_name;
    std::string args_type;
    std::string reply_type;

    std::string serializeToString() const {
        Serialize se(Serialize::SERIALIZER);
        se.writeString(service_name);
        se.writeString(method_name);
        se.writeString(args_type);
        se.writeString(reply_type);
        return se.toString();
    }

    static RPCMethod deserializeToRPCMethod(const std::string& str) {
        Serialize de(Serialize::DESERIALIZER, str);
        return {
            de.readString(),
            de.readString(),
            de.readString(),
            de.readString()
        };
    }
};

} // rpc
} // srpc

#endif
