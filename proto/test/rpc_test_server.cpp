#include "flog.h"
#include "eventloop.h"
#include "tcp_server.h"

#include "helloworld.h"

#include <functional>
#include <iostream>

class RPCServer : public TCPServer {
public:
    RPCServer(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions())
        : TCPServer(loop, addr, opts)
    {}

    // void regist_service(srpc::rpc::Service* service) {}
    void regist_service(const std::string& service_name, srpc::rpc::Service* service) {
        _mp[service_name] = service;
    }
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

    void skip_head_and_opt(std::string& str) {
        srpc::rpc::RPCPackage package;
        int header_len = sizeof(srpc::rpc::RPCHeader);

        srpc::rpc::RPCHeader header;
        ::memcpy(&header, str.data(), header_len);
        package.set_header(header);
        str.erase(0, header_len);
        if (package.has_opt()) {
            str.erase(0, RPC_OPTIONSIZE);
        }
    }

    srpc::rpc::RPCPackage get_package(Buffer& buffer) {
        return srpc::rpc::RPCPackage::from_string(buffer.retrieve_all());
    }

    srpc::rpc::RPCMethod get_method(const Buffer& buffer) {
        std::string bytes = buffer.peek_all();
        skip_head_and_opt(bytes);
        srpc::rpc::Serialize de(srpc::rpc::Serialize::DESERIALIZER, bytes);
        std::string service = de.readString();
        std::string method = de.readString();
        std::string args_type = de.readString();
        std::string reply_type = de.readString();
        srpc::rpc::RPCMethod rpc_method(service, method, args_type, reply_type);
        return rpc_method;
    }
    bool service_exist(const srpc::rpc::RPCMethod& rpc_method) {
        return _mp.find(rpc_method.service_name) != _mp.end();
    }
    srpc::rpc::RPCPackage process(const srpc::rpc::RPCMethod& rpc_method, const srpc::rpc::RPCPackage& pack) {
        srpc::rpc::RPCPackage ret_pack(srpc::rpc::RPCPackage::RPCResponse, srpc::rpc::RPCPackage::Local);
        if (service_exist(rpc_method)) {
            srpc::rpc::Service* service = _mp[rpc_method.service_name];
            return service->process(rpc_method, pack);
        } else {
            // 不存在该 rpc 方法
        }
        return ret_pack;
    }
private:
    std::map<std::string, srpc::rpc::Service*> _mp;
};

class GreeterServiceImpl : public Greeter::Service {
public:
    srpc::rpc::Status SayHello(const SayHelloArgs *args, SayHelloReply *reply) override {
        std::string prefix("Hello ");
        reply->reply = prefix + args->name;
        return {};
    }
};

int main() {
    using namespace std::placeholders;
    Logger::SetBufferLevel(Logger::kLineBuffer);
    EventLoop loop;
    Address addr("127.0.0.1", 8989);
    SocketOptions opt;
    opt.reuseaddr = true;
    RPCServer gs(&loop, addr, opt);
    GreeterServiceImpl service_impl;
    gs.regist_service("Greeter", &service_impl);
    gs.start();
    loop.loop();
    return 0;
}
