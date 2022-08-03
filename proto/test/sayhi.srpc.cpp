
#include "sayhi.srpc.h"
#include "socket.h"
#include "flog.h"
#include <memory>

MESSAGE_REGISTER(SayHiArgs);
    MESSAGE_REGISTER(SayHiReply);
    


Hi::Stub::Stub(const Address& addr)
    : _addr(addr)
    
    , _rpcmethod_SayHi("Hi", "SayHi", "SayHiArgs", "SayHiReply")
    
{}
    


srpc::rpc::Status Hi::Stub::SayHi(SayHiArgs *args, SayHiReply *reply) {
    srpc::rpc::Codeco codeco;
    auto local_args = std::make_shared<SayHiArgs>(*args); // copy construct
    bool is_args = true;
    auto msg = codeco.encoder(_rpcmethod_SayHi, local_args, srpc::rpc::RPCPackage::Local, is_args);

    TCPSocket sock;
    sock.connect(_addr);
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
    *reply = reply->deserializeToSayHiReply(response_pack.get_message()->to_string());
    return {};
}
    


Hi::Service::Service() {
    
    srpc::rpc::Service::add_method(new srpc::rpc::RPCServiceMethod(
                srpc::rpc::RPCMethod("Hi", "SayHi", "SayHiArgs", "SayHiReply"),
                new srpc::rpc::RPCMethodHandler<Hi::Service, SayHiArgs, SayHiReply,
                srpc::rpc::BaseMessage, srpc::rpc::BaseMessage>(
                    [](Hi::Service* service,
                       const SayHiArgs* args,
                       SayHiReply* reply) {
                    return service->SayHi(args, reply); }, this)));
    
}
    
    