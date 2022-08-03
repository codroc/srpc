#include "rpc/stub_generator.h"
#include <cassert>
using namespace rapidjson;

namespace srpc {
namespace rpc {

// 目前支持基础数据类型
// std::string
// bool
// int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
// double, float
// char

static std::map<std::string, std::string> WriteType = {
    {"std::string", R"(se.writeString([ParamName]);)"},
    {"bool", R"(se.writeVarInt8([ParamName]);)"},
    {"int8_t", R"(se.writeVarInt8([ParamName]);)"},
    {"int16_t", R"(se.writeVarInt16([ParamName]);)"},
    {"int32_t", R"(se.writeVarInt32([ParamName]);)"},
    {"int64_t", R"(se.writeVarInt64([ParamName]);)"},
    {"uint8_t", R"(se.writeUint8([ParamName]);)"},
    {"uint16_t", R"(se.writeVarUint16([ParamName]);)"},
    {"uint32_t", R"(se.writeVarUint32([ParamName]);)"},
    {"uint64_t", R"(se.writeVarUint64([ParamName]);)"},
    {"double", R"(se.writeFixed64([ParamName]);)"},
    {"float", R"(se.writeFixed32([ParamName]);)"},
    {"char", R"(se.writeUint8([ParamName]);)"},
};

static std::map<std::string, std::string> ReadType = {
    {"std::string", R"(de.readString(),)"},
    {"bool", R"(de.readVarInt8(),)"},
    {"int8_t", R"(de.readVarInt8(),)"},
    {"int16_t", R"(de.readVarInt16(),)"},
    {"int32_t", R"(de.readVarInt32(),)"},
    {"int64_t", R"(de.readVarInt64(),)"},
    {"uint8_t", R"(de.readUint8(),)"},
    {"uint16_t", R"(de.readVarUint16(),)"},
    {"uint32_t", R"(de.readVarUint32(),)"},
    {"uint64_t", R"(de.readVarUint64(),)"},
    {"double", R"(de.readFixed64(),)"},
    {"float", R"(de.readFixed32(),)"},
    {"char", R"(de.readUint8(),)"},
};

void StubGenerator::parse() {
    // 这是 rpc stub 生成类，因此规定 json value 中一定存在 "Message" 和 "Service" 对象
    assert(_value.IsObject());
    assert(_value.HasMember("Message"));
    assert(_value.HasMember("Service"));

    assert(_value["Message"].IsArray());
    assert(_value["Service"].IsArray());

    // _messages
    for (Value::ConstValueIterator it = _value["Message"].Begin(); it != _value["Message"].End(); ++it) {
        std::string msg_name = (*it)["Name"].GetString();
        assert((*it)["Param"].IsObject());
        std::vector<std::pair<std::string, std::string>> msg_params;
        for (auto inner_it = (*it)["Param"].MemberBegin(); inner_it != (*it)["Param"].MemberEnd(); ++inner_it) {
            msg_params.emplace_back(inner_it->name.GetString(), inner_it->value.GetString());   
        }
        _messages.emplace_back(msg_name, msg_params);
    }

    // _methods
    for (auto it = _value["Service"].Begin(); it != _value["Service"].End(); ++it) {
        std::string serv_name = (*it)["Name"].GetString();
        assert((*it)["Method"].IsArray());
        for (auto inner_it = (*it)["Method"].Begin(); inner_it != (*it)["Method"].End(); ++inner_it) {
            assert(inner_it->IsObject());
            _methods[serv_name].emplace_back(serv_name, (*inner_it)["Name"].GetString(),
                     (*inner_it)["ArgsType"].GetString(), (*inner_it)["ReplyType"].GetString());
        }
    }
}

std::string StubGenerator::generateHeaderFile(const std::string& filename) {
    std::string header_file = R"(
#ifndef SRPC_[Filename]_H
#define SRPC_[Filename]_H

#include "address.h"
#include "rpc/rpc.h"
#include "rpc/codeco.h"
#include "rpc/method_handler.h"
#include "rpc/message_register.h"
#include "rpc/message.h"
#include "rpc/serialize.h"
#include "rpc/rpc_status.h"
#include "rpc/service.h"

[MessageClasses]

[ServiceClasses]

#endif
    )";
    
    replaceAll(header_file, "[Filename]", filename + "_SRPC");
    replaceAll(header_file, "[MessageClasses]", generateMessageClass());
    replaceAll(header_file, "[ServiceClasses]", generateServiceClass());

    return header_file;
}
std::string StubGenerator::generateMessageClass() {
    std::string ret;
    std::string single_raw_args_class = R"(
class [ArgsTypeName] : public srpc::rpc::BaseMessage {
public:
    using ptr = std::shared_ptr<[ArgsTypeName]>;

    [ArgsTypeName]() = default;
    [ArgsTypeName]([ConstReferenceParams])
        // TODO: 构造函数里的参数要根据 data member 来设置
        // : name(name)
        : [InitializationList]
    {}
    ~[ArgsTypeName]() = default;

    std::string serializeToString() {
        srpc::rpc::Serialize se(srpc::rpc::Serialize::SERIALIZER);

        // TODO: 这里要根据类型中的 param 进行序列化，所以我得直到有哪些参数，以及参数的类型
        // se.writeString(name);
        [Serialize]

        return se.toString();
    }
    static [ArgsTypeName] deserializeTo[ArgsTypeName](const std::string& str) {
        srpc::rpc::Serialize de{srpc::rpc::Serialize::DESERIALIZER, str};
        return {
            
            // TODO: 这里要根据类型中的 param 进行反序列化，所以我得直到有哪些参数，以及参数的类型
            // de.readString()
            [Deserialize]

        };
    }

    virtual std::string to_string() override {
        return serializeToString();
    }
    virtual srpc::rpc::BaseMessage::ptr from_string(const std::string& str) override {
        return std::make_shared<[ArgsTypeName]>(std::move(deserializeTo[ArgsTypeName](str)));
    }

    virtual srpc::rpc::BaseMessage::ptr new_instance() override {
        return std::make_shared<[ArgsTypeName]>();
    }
public:
    // TODO: 这里要根据类型中的 param 进行设置，所以我得直到有哪些参数，以及参数的类型
    // 可能有多个参数
    // [TypeName] [ParamName];
[DataMember]
};
    )";

    std::string single_raw_datamember = R"(
        [TypeName] [ParamName];
    )";
    std::string single_raw_const_reference_param = R"([Type] const& [Param])";
    std::string single_raw_init = R"([Param]([Param]))";

    for (auto& msg : _messages) {
        std::string single_args_class = single_raw_args_class;
        std::string datamember;
        std::string serialize;
        std::string deserialize;

        std::string const_reference_params;
        std::string initialization_list;
        int i = 0;
        for (auto& p : msg.params) {
            // data member
            std::string single_datamember = single_raw_datamember;
            replaceAll(single_datamember, "[TypeName]", p.first);
            replaceAll(single_datamember, "[ParamName]", p.second);
            datamember += single_datamember;

            // serialize
            assert(WriteType.find(p.first) != WriteType.end());
            if (i > 0) serialize.push_back('\n');
            std::string seri = WriteType[p.first];
            replaceAll(seri, "[ParamName]", p.second);
            serialize += seri;

            // deserialize
            assert(ReadType.find(p.first) != ReadType.end());
            if (i > 0) deserialize.push_back('\n');
            deserialize += ReadType[p.first];

            // const_reference_params and initialization_list
            std::string single_const_reference_param = single_raw_const_reference_param;
            std::string single_init = single_raw_init;
            if (i++ > 0) {
                const_reference_params.append(", ");
                initialization_list.append(", ");
            }
            replaceAll(single_const_reference_param, "[Type]", p.first);
            replaceAll(single_const_reference_param, "[Param]", p.second);
            const_reference_params += single_const_reference_param;

            replaceAll(single_init, "[Param]", p.second);
            initialization_list += single_init;
        }
        replaceAll(single_args_class, "[ArgsTypeName]", msg.name);
        replaceAll(single_args_class, "[Serialize]", serialize);
        replaceAll(single_args_class, "[Deserialize]", deserialize);
        replaceAll(single_args_class, "[DataMember]", datamember);
        replaceAll(single_args_class, "[ConstReferenceParams]", const_reference_params);
        replaceAll(single_args_class, "[InitializationList]", initialization_list);

        ret += single_args_class;
    }

    return ret;
}

std::string StubGenerator::generateServiceClass() {
    std::string ret;

    std::string single_raw_service_class = R"(
class [ServiceName] {
public:
    class Stub {
    public:
        Stub(const Address& addr);
        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // srpc::rpc::Status [MethodName]([ArgsTypeName] *args, [ReplyTypeName] *reply);
        [ClientStubMemberFunctions]
    private:
        Address _addr;

        // TODO: 可能有多个方法
        // srpc::rpc::RPCMethod _rpcmethod_[MethodName];
        [DataMembers]
    };

    class Service : public srpc::rpc::Service {
    public:
        Service();

        // TODO: 这里我的知道所有的方法，以及传入方法的参数类型和返回值类型
        // 可能有多个方法
        // virtual srpc::rpc::Status [MethodName](const [ArgsTypeName] *args, [ReplyTypeName] *reply) = 0;
        [ServerStubMemberFunctions]
    };

    // TODO: 放一个序列化基类指针，然后具体的序列化方法就可以通过构造函数传入
    // Serializer* _se;

    static std::unique_ptr<Stub> NewStub(const Address& addr);
};
    )";

    std::string single_raw_client_function = R"(
        srpc::rpc::Status [MethodName]([ArgsTypeName] *args, [ReplyTypeName] *reply);
    )";
    std::string single_raw_datamember = R"(
        srpc::rpc::RPCMethod _rpcmethod_[MethodName];
    )";
    std::string single_raw_server_function = R"(
        virtual srpc::rpc::Status [MethodName](const [ArgsTypeName] *args, [ReplyTypeName] *reply) = 0;
    )";

    for (auto& [serv_name, methods] : _methods) {
        std::string single_service_class = single_raw_service_class;
        std::string client_functions;
        std::string server_functions;
        std::string datamembers;

        for (auto& method : methods) {
            // client_functions
            std::string single_client_function = single_raw_client_function;
            replaceAll(single_client_function, "[MethodName]", method.method_name);
            replaceAll(single_client_function, "[ArgsTypeName]", method.args_type);
            replaceAll(single_client_function, "[ReplyTypeName]", method.reply_type);
            client_functions += single_client_function;
            
            // server_functions
            std::string single_server_function = single_raw_server_function;
            replaceAll(single_server_function, "[MethodName]", method.method_name);
            replaceAll(single_server_function, "[ArgsTypeName]", method.args_type);
            replaceAll(single_server_function, "[ReplyTypeName]", method.reply_type);
            server_functions += single_server_function;

            // datamembers
            std::string single_datamember = single_raw_datamember;
            replaceAll(single_datamember, "[MethodName]", method.method_name);
            datamembers += single_datamember;
        }

        replaceAll(single_service_class, "[ServiceName]", serv_name);
        replaceAll(single_service_class, "[ClientStubMemberFunctions]", client_functions);
        replaceAll(single_service_class, "[ServerStubMemberFunctions]", server_functions);
        replaceAll(single_service_class, "[DataMembers]", datamembers);
        ret += single_service_class;
    }

    return ret;
}


std::string StubGenerator::generateSourceFile(const std::string& filename) {
    std::string source_file = R"(
#include "[Filename].h"
#include "socket.h"
#include "flog.h"
#include <memory>

[Macro]

[ServiceDefine]

[ClientStub]

[ServerStub]
    )";
    replaceAll(source_file, "[Filename]", filename + ".srpc");
    replaceAll(source_file, "[Macro]", generateMacro());
    replaceAll(source_file, "[ServiceDefine]", generateServiceDefine());
    replaceAll(source_file, "[ClientStub]", generateClientStubDefine());
    replaceAll(source_file, "[ServerStub]", generateServerStubDefine());

    return source_file;
}
std::string StubGenerator::generateMacro() {
    std::string ret;
    // 单行的类型注册宏
    std::string single_raw_macro = R"(MESSAGE_REGISTER([MessageName]);
    )";
    for (auto& msg : _messages) {
        std::string single_macro = single_raw_macro;
        replaceAll(single_macro, "[MessageName]", msg.name);
        ret += single_macro;
    }

    return ret;
}
std::string StubGenerator::generateServiceDefine() {
    std::string ret;
    std::string single_raw_service_define = R"(
[ServiceName]::Stub::Stub(const Address& addr)
    : _addr(addr)
    [RPCMethod]
{}
    )";
    std::string single_raw_rpcmethod = R"(
    , _rpcmethod_[MethodName]("[ServiceName]", "[MethodName]", "[ArgsTypeName]", "[ReplyTypeName]")
    )";

    for (auto &[serv_name, methods] : _methods) {
        std::string rpcmethods;
        for (auto& method : methods) {
            std::string tmp = single_raw_rpcmethod;
            replaceAll(tmp, "[MethodName]", method.method_name);
            replaceAll(tmp, "[ServiceName]", serv_name);
            replaceAll(tmp, "[ArgsTypeName]", method.args_type);
            replaceAll(tmp, "[ReplyTypeName]", method.reply_type);
            rpcmethods += tmp;
        }
        std::string tmp = single_raw_service_define;
        replaceAll(tmp, "[ServiceName]", serv_name);
        replaceAll(tmp, "[RPCMethod]", rpcmethods);
        ret += tmp;
    }

    return ret;
}
std::string StubGenerator::generateClientStubDefine() {
    std::string ret;
    std::string single_raw_client_stub = R"(
srpc::rpc::Status [ServiceName]::Stub::[MethodName]([ArgsTypeName] *args, [ReplyTypeName] *reply) {
    srpc::rpc::Codeco codeco;
    auto local_args = std::make_shared<[ArgsTypeName]>(*args); // copy construct
    bool is_args = true;
    auto msg = codeco.encoder(_rpcmethod_[MethodName], local_args, srpc::rpc::RPCPackage::Local, is_args);

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
    *reply = reply->deserializeTo[ReplyTypeName](response_pack.get_message()->to_string());
    return {};
}
    )";

    for (auto& [serv_name, methods] : _methods) {
        for (auto& method : methods) {
            std::string tmp = single_raw_client_stub;
            replaceAll(tmp, "[ServiceName]", serv_name);
            replaceAll(tmp, "[MethodName]", method.method_name);
            replaceAll(tmp, "[ArgsTypeName]", method.args_type);
            replaceAll(tmp, "[ReplyTypeName]", method.reply_type);
            ret += tmp;
        }
    }
        
    return ret;
}
std::string StubGenerator::generateServerStubDefine() {
    std::string ret;
    std::string single_raw_service_stub = R"(
[ServiceName]::Service::Service() {
    [AddMethod]
}
    )";
    std::string single_raw_addmethod = R"(
    srpc::rpc::Service::add_method(new srpc::rpc::RPCServiceMethod(
                srpc::rpc::RPCMethod("[ServiceName]", "[MethodName]", "[ArgsTypeName]", "[ReplyTypeName]"),
                new srpc::rpc::RPCMethodHandler<[ServiceName]::Service, [ArgsTypeName], [ReplyTypeName],
                srpc::rpc::BaseMessage, srpc::rpc::BaseMessage>(
                    []([ServiceName]::Service* service,
                       const [ArgsTypeName]* args,
                       [ReplyTypeName]* reply) {
                    return service->[MethodName](args, reply); }, this)));
    )";

    for (auto& [serv_name, methods] : _methods) {
        std::string tmp = single_raw_service_stub;
        std::string addmethods;
        for (auto& method : methods) {
            std::string inner_tmp = single_raw_addmethod;
            replaceAll(inner_tmp, "[ServiceName]", serv_name);
            replaceAll(inner_tmp, "[MethodName]", method.method_name);
            replaceAll(inner_tmp, "[ArgsTypeName]", method.args_type);
            replaceAll(inner_tmp, "[ReplyTypeName]", method.reply_type);
            addmethods += inner_tmp;
        }
        replaceAll(tmp, "[ServiceName]", serv_name);
        replaceAll(tmp, "[AddMethod]", addmethods);
        ret += tmp;
    }

    return ret;
}

void StubGenerator::replaceAll(std::string& raw_string, const std::string& src, const std::string& dest) {
    std::string::size_type pos = 0;
    while (pos != std::string::npos) {
        pos = raw_string.find(src, pos);
        if (pos == std::string::npos)
            break;
        raw_string.replace(pos, src.size(), dest);
    }
}

}
}
