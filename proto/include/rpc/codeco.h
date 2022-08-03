#ifndef SRPC_PROTO_RPC_CODECO_H
#define SRPC_PROTO_RPC_CODECO_H

#include "rpc/rpc.h"
#include "rpc/message.h"

namespace srpc {
namespace rpc {

// serialization type: Local
// 为了从 RPCPackage 中获取 RPCMethod 以及 args/reply
class Codeco {
public:
    // RPCPackage encoder(const RPCMethod& method, BaseMessage* msg, RPCPackage::SerializationType type);
    std::string encoder(const RPCMethod& method, BaseMessage::ptr msg, RPCPackage::SerializationType type, bool is_args);
    // RPCMethod decoder(const RPCPackage& package);
    // 这里的 bytestream 已经是一个 package 包序列化后的 bytes 了，不存在粘包半包的情况
    RPCPackage decoder(const std::string& bytestream);
private:
    uint16_t calculate_checksum() { return 0; }
    bool is_rpc_package(const std::string&);
    bool has_bit_error(const RPCPackage&);
};

} // rpc
} // srpc

#endif
