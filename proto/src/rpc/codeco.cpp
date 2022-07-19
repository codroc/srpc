#include "codeco.h"

namespace srpc {
namespace rpc {


bool Codeco::is_rpc_package(const RPCPackage&) {
    return true;
}
bool Codeco::has_bit_error(const RPCPackage&) {
    return false;
}

RPCPackage Codeco::encoder(const RPCMethod& method, BaseMessage* msg, RPCPackage::SerializationType type) {
    RPCPackage package;
    package.set_serialization_type(type);
    if (!msg) {
        package.set_heartbeat();
    } else if (msg->is_args()) {
        package.set_request();
    } else if (msg->is_reply()) {
        package.set_response();
    }

    package.set_body(method.serializeToString() + msg->serialize());
    package.set_checksum();
    return package;
}

RPCMethod Codeco::decoder(const RPCPackage& package) {
    if (!is_rpc_package(package) || has_bit_error(package)) {
        return {};
    }

    return RPCMethod::deserializeToRPCMethod(package.get_body());
}

}
}
