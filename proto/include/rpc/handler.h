#ifndef SRPC_PROTO_RPC_HANDLER_H
#define SRPC_PROTO_RPC_HANDLER_H

#include "rpc.h"
#include "rpc_status.h"
#include <functional>
namespace srpc {
namespace rpc {

class MethodHandler {
public:
    MethodHandler() = default;
    virtual ~MethodHandler() = default;

    struct HandlerParameter {
        HandlerParameter(const RPCMethod& rm, void* parg)
            : rpc_method(rm), arg(parg)
        {}
        ~HandlerParameter() {}
        void* const arg;
        RPCMethod rpc_method;
    };

    virtual RPCPackage run_handler(const HandlerParameter& param) = 0;
};

template <class ServiceType, class ArgType, class ReplyType,
          class BaseArgType = ArgType,
          class BaseReplyType = ReplyType>
class RPCMethodHandler : public MethodHandler {
public:
    RPCMethodHandler(
            std::function<Status(ServiceType*, const ArgType*, ReplyType*)>
            func, ServiceType* service)
        : _func(func), _service(service) {}
    RPCPackage run_handler(const HandlerParameter& param) override {
        ReplyType reply;
        Status status = _func(_service, static_cast<ArgType*>(param.arg), &reply);

        RPCPackage pack(RPCPackage::RPCResponse, RPCPackage::Local);
        if (status.ok()) {
            pack.set_body(param.rpc_method.serializeToString() + reply.serialize());
            pack.set_status(RPCPackage::Ok);
        } else {
            pack.set_status(RPCPackage::Error);
        }
        return pack;
    }
private:
    std::function<Status(ServiceType*, const ArgType*, ReplyType*)> _func;
    ServiceType* _service;
};

}
}

#endif
