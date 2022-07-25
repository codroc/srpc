#include "rpc/rpc_server.h"
#include <string>

namespace srpc {
namespace rpc {

RPCServer::RPCServer(EventLoop* loop, Address addr, SocketOptions opts)
    : TCPServer(loop, addr, opts)
{}

void RPCServer::regist_service(const std::string& service_name, Service* service) {
    _mp[service_name] = service;
}

void RPCServer::OnConnection(TCPConnection::ptr conn) {}

void RPCServer::OnMessage(TCPConnection::ptr conn, Buffer& buffer) {
    if (is_complete_package(buffer)) {
        auto pack = get_package(buffer);
        // TODO: stub 去解析 RPC 包...
        // 这里需要设计下调用的接口
        auto ret_pack = process(pack);
        conn->send(ret_pack.to_string());
        conn->force_close();
    }
};

bool RPCServer::is_complete_package(Buffer& buffer) {
    // fix me:
    return true;
}

RPCPackage RPCServer::get_package(Buffer& buffer) {
    return RPCPackage::from_string(buffer.retrieve_all());
}

bool RPCServer::service_exist(const RPCMethod& rpc_method) {
    return _mp.find(rpc_method.service_name) != _mp.end();
}

RPCPackage RPCServer::process(const RPCPackage& pack) {
    RPCPackage ret_pack(RPCPackage::RPCResponse, RPCPackage::Local);
    if (service_exist(pack.get_rpc_method())) {
        Service* service = _mp[pack.get_rpc_method().service_name];
        return service->process(pack);
    } else {
        // 不存在该 rpc 方法
    }
    return ret_pack;
}

}
}
