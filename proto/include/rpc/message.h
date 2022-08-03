#ifndef SRPC_PROTO_RPC_MESSAGE_H
#define SRPC_PROTO_RPC_MESSAGE_H
#include <memory>
#include <map>

namespace srpc {
namespace rpc {

class BaseMessage {
public:
    using ptr = std::shared_ptr<BaseMessage>;
    virtual std::string to_string() = 0;
    virtual BaseMessage::ptr from_string(const std::string& str) = 0;
    virtual BaseMessage::ptr new_instance() = 0;
};

std::map<std::string, BaseMessage::ptr>& GetInstanceMap();

BaseMessage::ptr MessageFactory(const std::string& type_name);

} // namespace rpc
} // namespace srpc

#endif
