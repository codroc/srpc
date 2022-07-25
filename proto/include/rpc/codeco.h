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
    RPCPackage encoder(const RPCMethod& method, BaseMessage* msg, RPCPackage::SerializationType type);
    RPCMethod decoder(const RPCPackage& package);
private:
    uint16_t calculate_checksum() { return 0; }
    bool is_rpc_package(const RPCPackage&);
    bool has_bit_error(const RPCPackage&);
};

} // rpc
} // srpc

#endif
