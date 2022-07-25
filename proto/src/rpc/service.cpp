#include "rpc/service.h"
#include "rpc/method_handler.h"
#include "rpc/message.h"

namespace srpc {
namespace rpc {

RPCPackage Service::process(const RPCPackage& pack) {
    auto rpc_method = pack.get_rpc_method();
    auto handler = _methods[rpc_method.method_name]->handler();

    return handler->run_handler(MethodHandler::HandlerParameter(rpc_method, pack.get_message()));
}
void Service::add_method(RPCServiceMethod* service_method) {
    std::unique_ptr<RPCServiceMethod> up(service_method);
    _methods[service_method->method_name()] = std::move(up);
}

}
}
