#include "flog.h"

#include "rpc/rpc_server.h"
#include "sayhi.srpc.h"

class HiServiceImpl : public Hi::Service {
public:
    virtual srpc::rpc::Status SayHi(const SayHiArgs *args, SayHiReply *reply) override {
        std::string prefix("Hello ");
        reply->reply = prefix + args->name;
        return {};
    }
};

int main() {
    Logger::SetBufferLevel(Logger::kLineBuffer);

    EventLoop loop;
    Address addr("0.0.0.0", 12345);
    SocketOptions opt;
    opt.reuseaddr = true;
    srpc::rpc::RPCServer gs(&loop, addr, opt);

    HiServiceImpl service_impl;
    gs.regist_service("Hi", &service_impl);
    gs.start();

    loop.loop();
    return 0;
}

