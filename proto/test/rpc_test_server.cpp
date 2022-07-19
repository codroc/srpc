#include "flog.h"
#include "eventloop.h"
#include "tcp_server.h"
#include "helloworld.h"
#include "service.h"
#include <functional>

class RPCServer : public TCPServer {
public:
    using MethodFunc = std::function<srpc::rpc::Status(srpc::rpc::BaseMessage*, srpc::rpc::BaseMessage*)>;
    RPCServer(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions())
        : TCPServer(loop, addr, opts)
    {}

    void regist_service(srpc::rpc::Service* service) {}

    virtual void OnConnection(TCPConnection::ptr conn) override {
    };
    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) override {
        if (is_complete_package(buffer)) {
            auto method = get_method(buffer);
            auto pack = get_package(buffer);
            // TODO: stub 去解析 RPC 包...
            // 这里需要设计下调用的接口
            auto ret_pack = process(method, pack);
            conn->send(ret_pack.to_string());
            conn->force_close();
        }
    };
private:
    bool is_complete_package(Buffer& buffer) {
        // fix me:
        return true;
    }

    srpc::rpc::RPCPackage get_package(Buffer& buffer) {
        return srpc::rpc::RPCPackage::from_string(buffer.retrieve_all());
    }
    srpc::rpc::RPCMethod get_method(const Buffer& buffer) {
        return {};
    }
    srpc::rpc::RPCPackage process(const srpc::rpc::RPCMethod& method, const srpc::rpc::RPCPackage& pack) {
        return pack;
    }
private:
    std::map<std::string, MethodFunc> _mp;
};

class GreeterServiceImpl : public Greeter::Service {
public:
    srpc::rpc::Status SayHello(SayHelloArgs *args, SayHelloReply *reply) override {
        std::string prefix("Hello ");
        reply->reply = prefix + args->name;
        return {};
    }
};

int main() {
    Logger::SetBufferLevel(Logger::kLineBuffer);
    EventLoop loop;
    Address addr("127.0.0.1", 8989);
    SocketOptions opt;
    opt.reuseaddr = true;
    RPCServer gs(&loop, addr, opt);
    gs.start();
    loop.loop();
    return 0;
}
