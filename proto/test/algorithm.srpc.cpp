
#include "algorithm.srpc.h"
#include "socket.h"
#include "flog.h"
#include <memory>

MESSAGE_REGISTER(AddArgs);
    MESSAGE_REGISTER(AddReply);
    MESSAGE_REGISTER(ModArgs);
    MESSAGE_REGISTER(ModReply);
    


Cal::Stub::Stub(const Address& addr)
    : _addr(addr)
    
    , _rpcmethod_Add("Cal", "Add", "AddArgs", "AddReply")
    
    , _rpcmethod_Mod("Cal", "Mod", "ModArgs", "ModReply")
    
{}
    


srpc::rpc::Status Cal::Stub::Add(AddArgs *args, AddReply *reply) {
    srpc::rpc::Codeco codeco;
    auto local_args = std::make_shared<AddArgs>(*args); // copy construct
    bool is_args = true;
    auto msg = codeco.encoder(_rpcmethod_Add, local_args, srpc::rpc::RPCPackage::Local, is_args);

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
    *reply = reply->deserializeToAddReply(response_pack.get_message()->to_string());
    return {};
}
    
srpc::rpc::Status Cal::Stub::Mod(ModArgs *args, ModReply *reply) {
    srpc::rpc::Codeco codeco;
    auto local_args = std::make_shared<ModArgs>(*args); // copy construct
    bool is_args = true;
    auto msg = codeco.encoder(_rpcmethod_Mod, local_args, srpc::rpc::RPCPackage::Local, is_args);

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
    *reply = reply->deserializeToModReply(response_pack.get_message()->to_string());
    return {};
}
    


Cal::Service::Service() {
    
    srpc::rpc::Service::add_method(new srpc::rpc::RPCServiceMethod(
                srpc::rpc::RPCMethod("Cal", "Add", "AddArgs", "AddReply"),
                new srpc::rpc::RPCMethodHandler<Cal::Service, AddArgs, AddReply,
                srpc::rpc::BaseMessage, srpc::rpc::BaseMessage>(
                    [](Cal::Service* service,
                       const AddArgs* args,
                       AddReply* reply) {
                    return service->Add(args, reply); }, this)));
    
    srpc::rpc::Service::add_method(new srpc::rpc::RPCServiceMethod(
                srpc::rpc::RPCMethod("Cal", "Mod", "ModArgs", "ModReply"),
                new srpc::rpc::RPCMethodHandler<Cal::Service, ModArgs, ModReply,
                srpc::rpc::BaseMessage, srpc::rpc::BaseMessage>(
                    [](Cal::Service* service,
                       const ModArgs* args,
                       ModReply* reply) {
                    return service->Mod(args, reply); }, this)));
    
}
    
    