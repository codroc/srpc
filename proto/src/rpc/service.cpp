#include "rpc/service.h"
#include "rpc/method_handler.h"
#include "rpc/message.h"

namespace srpc {
namespace rpc {

RPCPackage Service::process(const RPCMethod& rpc_method, const RPCPackage& pack) {
    auto handler = _methods[rpc_method.method_name]->handler();

    std::string str_arg = pack.get_arg_or_reply();
    auto arg = GetInstanceMap()[rpc_method.args_type].get()->new_instance()->deserialize(str_arg);

    return handler->run_handler(MethodHandler::HandlerParameter(rpc_method, arg.get()));
}
void Service::add_method(RPCServiceMethod* service_method) {
    std::unique_ptr<RPCServiceMethod> up(service_method);
    _methods[service_method->method_name()] = std::move(up);
}

}
}
