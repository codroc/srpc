#ifndef HELLOWORLD_H
#define HELLOWORLD_H
#include "address.h"
#include "rpc/rpc.h"
#include "rpc/codeco.h"
#include "rpc/method_handler.h"
#include "rpc/message_register.h"
#include "rpc/message.h"
#include "rpc/serialize.h"
#include "rpc/rpc_status.h"
#include "rpc/service.h"

class SayHelloArgs : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<SayHelloArgs>;

    SayHelloArgs() = default;
    SayHelloArgs(const std::string& name)
        : name(name) {}
    ~SayHelloArgs() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);
        se.writeString(name);
        return se.toString();
    }
    static SayHelloArgs deserializeToSayHelloArgs(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {de.readString()};
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<SayHelloArgs>(std::move(deserializeToSayHelloArgs(str)));
    }

    bool is_args() const override { return true; }
    bool is_reply() const override { return false; }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<SayHelloArgs>();
    }
public:
    std::string name;
};

class SayHelloReply : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<SayHelloReply>;

    SayHelloReply() = default;
    SayHelloReply(const std::string& reply)
        : reply(reply) {}
    ~SayHelloReply() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);
        se.writeString(reply);
        return se.toString();
    }
    static SayHelloReply deserializeToSayHelloReply(const std::string& rpc_body) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, rpc_body};
        return {de.readString()};
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& rpc_body) override {
        return std::make_shared<SayHelloReply>(std::move(deserializeToSayHelloReply(rpc_body)));
    }

    bool is_args() const override { return false; }
    bool is_reply() const override { return true; }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<SayHelloReply>();
    }
public:
    std::string reply;
};

class Greeter {
public:
    class Stub {
    public:
        Stub(const Address& addr);
        srpc::rpc::Status SayHello(SayHelloArgs *args, SayHelloReply *reply);
    private:
        Address _addr;
        srpc::rpc::RPCMethod _rpcmethod_SayHello;
    };

    class Service : public srpc::rpc::Service {
    public:
        Service();
        virtual srpc::rpc::Status SayHello(const SayHelloArgs *args, SayHelloReply *reply) = 0;
    };

    // TODO: 放一个序列化基类指针，然后具体的序列化方法就可以通过构造函数传入
    // Serializer* _se;

    static std::unique_ptr<Stub> NewStub(const Address& addr);
};

#endif
