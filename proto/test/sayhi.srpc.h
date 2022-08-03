
#ifndef SRPC_sayhi_SRPC_H
#define SRPC_sayhi_SRPC_H

#include "address.h"
#include "rpc/rpc.h"
#include "rpc/codeco.h"
#include "rpc/method_handler.h"
#include "rpc/message_register.h"
#include "rpc/message.h"
#include "rpc/serialize.h"
#include "rpc/rpc_status.h"
#include "rpc/service.h"


class SayHiArgs : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<SayHiArgs>;

    SayHiArgs() = default;
    SayHiArgs(std::string const& name)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : name(name)
    {}
    ~SayHiArgs() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeString(name);

        return se.toString();
    }
    static SayHiArgs deserializeToSayHiArgs(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readString(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<SayHiArgs>(std::move(deserializeToSayHiArgs(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<SayHiArgs>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        std::string name;
    
};
    
class SayHiReply : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<SayHiReply>;

    SayHiReply() = default;
    SayHiReply(std::string const& reply)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : reply(reply)
    {}
    ~SayHiReply() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeString(reply);

        return se.toString();
    }
    static SayHiReply deserializeToSayHiReply(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readString(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<SayHiReply>(std::move(deserializeToSayHiReply(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<SayHiReply>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        std::string reply;
    
};
    


class Hi {
public:
    class Stub {
    public:
        Stub(const Address& addr);
        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // srpc::rpc::Status [MethodName]([ArgsTypeName] *args, [ReplyTypeName] *reply);
        
        srpc::rpc::Status SayHi(SayHiArgs *args, SayHiReply *reply);
    
    private:
        Address _addr;

        // TODO: 可能有多个方法
        // srpc::rpc::RPCMethod _rpcmethod_[MethodName];
        
        srpc::rpc::RPCMethod _rpcmethod_SayHi;
    
    };

    class Service : public srpc::rpc::Service {
    public:
        Service();

        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // virtual srpc::rpc::Status [MethodName](const [ArgsTypeName] *args, [ReplyTypeName] *reply) = 0;
        
        virtual srpc::rpc::Status SayHi(const SayHiArgs *args, SayHiReply *reply) = 0;
    
    };

    // TODO: 放一个序列化基类指针，然后具体的序列化方法就可以通过构造函数传入
    // Serializer* _se;

    static std::unique_ptr<Stub> NewStub(const Address& addr);
};
    

#endif
    