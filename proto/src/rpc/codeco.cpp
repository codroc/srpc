#include "rpc/codeco.h"

namespace srpc {
namespace rpc {


bool Codeco::is_rpc_package(const std::string& bytestream) {
    return true;
}
bool Codeco::has_bit_error(const RPCPackage&) {
    return false;
}

std::string Codeco::encoder(const RPCMethod& method, BaseMessage::ptr msg, RPCPackage::SerializationType type) {
    RPCPackage package;
    package.set_serialization_type(type);
    if (!msg) {
        package.set_heartbeat();
        return package.to_string(); // 心跳包不填 checksum 也可以
    } else if (msg->is_args()) {
        package.set_request();
    } else if (msg->is_reply()) {
        package.set_response();
    }

    package.set_rpc_method(method);
    package.set_message(msg);
    package.set_checksum();
    return package.to_string();
}

RPCPackage Codeco::decoder(const std::string& bytestream) {
    if (!is_rpc_package(bytestream)) {
        return {};
    }
    auto package = RPCPackage::from_string(bytestream);
    if (has_bit_error(package)) {
        return {};
    }

    return package;
}

}
}
