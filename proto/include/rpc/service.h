#ifndef SRPC_PROTO_RPC_SERVICE_H
#define SRPC_PROTO_RPC_SERVICE_H

#include "rpc.h"
#include "handler.h"
#include <map>
#include <string>
#include <memory>

namespace srpc {
namespace rpc {

class RPCServiceMethod {
public:
    RPCServiceMethod(const RPCMethod& rpc_method, MethodHandler* handler)
        : _rpc_method(rpc_method), _handler(handler)
    {}
    const std::string& method_name() const { return _rpc_method.method_name; }
    MethodHandler* handler() { return _handler.get(); }
private:
    RPCMethod _rpc_method;
    std::unique_ptr<MethodHandler> _handler;
};
class Service {
public:
    RPCPackage process(const RPCMethod& rpc_method, const RPCPackage& pack);
    void add_method(RPCServiceMethod* service_method);
private:
    std::map<std::string, std::unique_ptr<RPCServiceMethod>> _methods;
};

}
}
#endif
