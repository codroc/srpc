#ifndef SRPC_PROTO_RPC_MESSAGE_H
#define SRPC_PROTO_RPC_MESSAGE_H
#include <memory>
#include <map>

namespace srpc {
namespace rpc {

class BaseMessage {
public:
    using ptr = std::shared_ptr<BaseMessage>;
    virtual std::string serialize() = 0;
    virtual BaseMessage::ptr deserialize(const std::string& str) = 0;

    virtual bool is_args() const = 0;
    virtual bool is_reply() const = 0;
};

std::map<std::string, BaseMessage::ptr>& GetInstanceMap();

} // namespace rpc
} // namespace srpc

#endif
