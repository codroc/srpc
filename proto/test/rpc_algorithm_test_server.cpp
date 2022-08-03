#include "flog.h"

#include "rpc/rpc_server.h"
#include "algorithm.srpc.h"

class CalServiceImpl : public Cal::Service {
public:
    virtual srpc::rpc::Status Add(const AddArgs *args, AddReply *reply) override {
        reply->res = args->a + args->b;
        return {};
    }
    virtual srpc::rpc::Status Mod(const ModArgs *args, ModReply *reply) override {}
};

int main() {
    Logger::SetBufferLevel(Logger::kLineBuffer);

    EventLoop loop;
    Address addr("127.0.0.1", 6677);
    SocketOptions opt;
    opt.reuseaddr = true;
    srpc::rpc::RPCServer gs(&loop, addr, opt);

    CalServiceImpl service_impl;
    gs.regist_service("Cal", &service_impl);
    gs.start();

    loop.loop();
    return 0;
}

