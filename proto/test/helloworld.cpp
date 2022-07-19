#include "helloworld.h"
#include "message.h"
#include "register.h"
#include "codeco.h"
#include "socket.h"
#include "flog.h"

// static const char* Greeter_method_name[] = {
//     "Greeter.SayHello"
// };

TYPE_REGISTER(SayHelloArgs);
TYPE_REGISTER(SayHelloReply);

Greeter::Stub::Stub(const Address& addr)
    : _addr(addr)
    , _rpcmethod_SayHello("Greeter", "SayHello", "SayHelloArgs", "SayHelloReply")
{}

// 这个是同步调用
srpc::rpc::Status Greeter::Stub::SayHello(SayHelloArgs *args, SayHelloReply *reply) {
    srpc::rpc::Codeco codeco;
    auto package = codeco.encoder(_rpcmethod_SayHello, args, srpc::rpc::RPCPackage::Local);
    TCPSocket sock;
    sock.connect(_addr);
    std::string msg = package.to_string();
    ssize_t written = sock.send(msg);
    // fix me:
    if (written != msg.size()) {
        LOG_WARN << "Not implement: out bound buffer is not enough!\n";
        exit(1);
    }
    std::string readed;
    std::string ret;
    while (sock.recv(readed) != 0) {
        ret += readed;
        readed.clear();
    }
    
    auto response_pack = srpc::rpc::RPCPackage::from_string(ret);
    set_reply(response_pack, reply);
    return {};
}

void Greeter::Stub::set_reply(srpc::rpc::RPCPackage pack, SayHelloReply* reply) {
    std::string body = pack.get_body();
    *reply = reply->deserializeToSayHelloReply(body);
}
