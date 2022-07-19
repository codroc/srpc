#ifndef HELLOWORLD_H
#define HELLOWORLD_H
#include "address.h"
#include "rpc.h"
#include "message.h"
#include "serialize.h"
#include "rpc_status.h"
#include "service.h"

class SayHelloArgs : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<SayHelloArgs>;

    SayHelloArgs() = default;
    SayHelloArgs(const std::string& name)
        : name(name) {}
    ~SayHelloArgs() = default;

    std::string serializeToString() {
        Serialize se(Serialize::SERIALIZER);
        se.writeString(name);
        return se.toString();
    }
    static SayHelloArgs deserializeToSayHelloArgs(const std::string& str) {
        Serialize de{Serialize::DESERIALIZER, str};
        return {de.readString()};
    }

    std::string serialize() override {
        return serializeToString();
    }
    srpc::rpc::BaseMessage::ptr deserialize(const std::string& str) override {
        return std::make_shared<SayHelloArgs>(std::move(deserializeToSayHelloArgs(str)));
    }

    bool is_args() const override { return true; }
    bool is_reply() const override { return false; }

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
        Serialize se(Serialize::SERIALIZER);
        se.writeString(reply);
        return se.toString();
    }
    static SayHelloReply deserializeToSayHelloReply(const std::string& rpc_body) {
        Serialize de{Serialize::DESERIALIZER, rpc_body};
        std::string service_name = de.readString();
        std::string method_name = de.readString();
        std::string args_type = de.readString();
        std::string reply_type = de.readString();
        return {de.readString()};
    }

    std::string serialize() {
        return serializeToString();
    }
    srpc::rpc::BaseMessage::ptr deserialize(const std::string& rpc_body) override {
        return std::make_shared<SayHelloReply>(std::move(deserializeToSayHelloReply(rpc_body)));
    }

    bool is_args() const override { return false; }
    bool is_reply() const override { return true; }

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
        void set_reply(srpc::rpc::RPCPackage pack, SayHelloReply* reply);
    private:
        Address _addr;
        srpc::rpc::RPCMethod _rpcmethod_SayHello;
    };

    class Service : public srpc::rpc::Service {
    public:
        virtual srpc::rpc::Status SayHello(SayHelloArgs *args, SayHelloReply *reply);
    };

    static std::unique_ptr<Stub> NewStub(const Address& addr);
private:
    std::string _service_name;
};

#endif
