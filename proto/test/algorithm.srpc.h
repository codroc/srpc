
#ifndef SRPC_algorithm_SRPC_H
#define SRPC_algorithm_SRPC_H

#include "address.h"
#include "rpc/rpc.h"
#include "rpc/codeco.h"
#include "rpc/method_handler.h"
#include "rpc/message_register.h"
#include "rpc/message.h"
#include "rpc/serialize.h"
#include "rpc/rpc_status.h"
#include "rpc/service.h"


class AddArgs : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<AddArgs>;

    AddArgs() = default;
    AddArgs(int32_t const& a, int32_t const& b)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : a(a), b(b)
    {}
    ~AddArgs() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeVarInt32(a);
se.writeVarInt32(b);

        return se.toString();
    }
    static AddArgs deserializeToAddArgs(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readVarInt32(),
de.readVarInt32(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<AddArgs>(std::move(deserializeToAddArgs(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<AddArgs>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        int32_t a;
    
        int32_t b;
    
};
    
class AddReply : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<AddReply>;

    AddReply() = default;
    AddReply(int32_t const& res)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : res(res)
    {}
    ~AddReply() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeVarInt32(res);

        return se.toString();
    }
    static AddReply deserializeToAddReply(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readVarInt32(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<AddReply>(std::move(deserializeToAddReply(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<AddReply>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        int32_t res;
    
};
    
class ModArgs : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<ModArgs>;

    ModArgs() = default;
    ModArgs(int32_t const& a, int32_t const& b)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : a(a), b(b)
    {}
    ~ModArgs() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeVarInt32(a);
se.writeVarInt32(b);

        return se.toString();
    }
    static ModArgs deserializeToModArgs(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readVarInt32(),
de.readVarInt32(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<ModArgs>(std::move(deserializeToModArgs(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<ModArgs>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        int32_t a;
    
        int32_t b;
    
};
    
class ModReply : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<ModReply>;

    ModReply() = default;
    ModReply(int32_t const& res)
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : res(res)
    {}
    ~ModReply() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        se.writeVarInt32(res);

        return se.toString();
    }
    static ModReply deserializeToModReply(const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            de.readVarInt32(),

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<ModReply>(std::move(deserializeToModReply(str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<ModReply>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];

        int32_t res;
    
};
    


class Cal {
public:
    class Stub {
    public:
        Stub(const Address& addr);
        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // srpc::rpc::Status [MethodName]([ArgsTypeName] *args, [ReplyTypeName] *reply);
        
        srpc::rpc::Status Add(AddArgs *args, AddReply *reply);
    
        srpc::rpc::Status Mod(ModArgs *args, ModReply *reply);
    
    private:
        Address _addr;

        // TODO: 可能有多个方法
        // srpc::rpc::RPCMethod _rpcmethod_[MethodName];
        
        srpc::rpc::RPCMethod _rpcmethod_Add;
    
        srpc::rpc::RPCMethod _rpcmethod_Mod;
    
    };

    class Service : public srpc::rpc::Service {
    public:
        Service();

        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // virtual srpc::rpc::Status [MethodName](const [ArgsTypeName] *args, [ReplyTypeName] *reply) = 0;
        
        virtual srpc::rpc::Status Add(const AddArgs *args, AddReply *reply) = 0;
    
        virtual srpc::rpc::Status Mod(const ModArgs *args, ModReply *reply) = 0;
    
    };

    // TODO: 放一个序列化基类指针，然后具体的序列化方法就可以通过构造函数传入
    // Serializer* _se;

    static std::unique_ptr<Stub> NewStub(const Address& addr);
};
    

#endif
    