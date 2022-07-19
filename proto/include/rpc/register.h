#ifndef SRPC_PROTO_RPC_REGISTER_H
#define SRPC_PROTO_RPC_REGISTER_H

#define TYPE_REGISTER(message_name) \
struct register_##message_name { \
    register_##message_name() { \
        srpc::rpc::GetInstanceMap()[#message_name] = std::make_shared<message_name>(); \
    } \
}; \
register_##message_name register_##message_name##_instance

#endif
