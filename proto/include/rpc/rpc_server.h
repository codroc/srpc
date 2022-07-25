#ifndef SRPC_PROTO_RPC_RPC_SERVER_H
#define SRPC_PROTO_RPC_RPC_SERVER_H

#include "eventloop.h"
#include "address.h"
#include "socket.h"
#include "tcp_server.h"

#include "rpc/rpc.h"
#include "rpc/service.h"


namespace srpc {
namespace rpc {

class RPCServer : public TCPServer {
public:
    RPCServer(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions());

    void regist_service(const std::string& service_name, Service* service);

    virtual void OnConnection(TCPConnection::ptr conn) override;

    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) override;
private:
    bool is_complete_package(Buffer& buffer);

    RPCPackage get_package(Buffer& buffer);

    bool service_exist(const RPCMethod& rpc_method);

    RPCPackage process(const RPCPackage& pack);
private:
    std::map<std::string, Service*> _mp;
};

}
}

#endif
