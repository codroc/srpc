#include "flog.h"

#include "rpc/rpc_server.h"
#include "helloworld.h"

class GreeterServiceImpl : public Greeter::Service {
public:
    virtual srpc::rpc::Status SayHello(const SayHelloArgs *args, SayHelloReply *reply) override {
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
    srpc::rpc::RPCServer gs(&loop, addr, opt);

    GreeterServiceImpl service_impl;
    gs.regist_service("Greeter", &service_impl);
    gs.start();

    loop.loop();
    return 0;
}
