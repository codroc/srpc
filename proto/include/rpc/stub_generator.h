#ifndef SRPC_PROTO_RPC_STUB_GENERATOR_H
#define SRPC_PROTO_RPC_STUB_GENERATOR_H

#include <rapidjson/document.h>

namespace srpc {
namespace rpc {

class StubGenerator {
public:
    StubGenerator(rapidjson::Value value)
        : _value(value) 
    {
        parse(_value);
    }
    ~StubGenerator() = default;

    std::string generateHeaderFile();
    std::string generateArgsClass();
    std::string generateReplyClass();
    std::string generateServiceClass();

    std::string generateSourceFile();
    std::string generateMacro();
    std::string generateServiceDefine();
    std::string generateClientStubDefine();
    std::string generateServiceStubDefine();
public:
    struct Message {
        std::string name;
        std::vector<pair<std::string, std::string>> params;
    };
private:
    // parse json value, and fill up member data
    void parse();

    rapidjson::Value _value;
    std::vector<Message> _messages; // 包括 args type 和 reply type
    std::vector<RPCMethod> _methods;
};

}
}

#endif
