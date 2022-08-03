#ifndef SRPC_PROTO_RPC_STUB_GENERATOR_H
#define SRPC_PROTO_RPC_STUB_GENERATOR_H

#include "rpc/rpc.h"
#include <rapidjson/document.h>
#include <vector>
#include <string>
#include <map>

namespace srpc {
namespace rpc {

class StubGenerator {
public:
    StubGenerator(const std::string& output_filename, rapidjson::Document& value)
        : _output_filename(output_filename)
    {
        _value.CopyFrom(value, value.GetAllocator());
        parse();
    }
    ~StubGenerator() = default;

    std::string generateHeaderFile(const std::string& filename);
    std::string generateMessageClass();
    std::string generateServiceClass();

    std::string generateSourceFile(const std::string& filename);
    std::string generateMacro();
    std::string generateServiceDefine();
    std::string generateClientStubDefine();
    std::string generateServerStubDefine();
public:
    struct Message {
        Message(const std::string& n, const std::vector<std::pair<std::string, std::string>>& p)
            : name(n), params(p)
        {}
        std::string name;
        std::vector<std::pair<std::string, std::string>> params;
    };
private:
    // parse json value, and fill up member data
    void parse();
    // for test:
    void waithere() {
        while (1) {}
    }
    // replace all src from raw_string to dest
    void replaceAll(std::string& raw_string, const std::string& src, const std::string& dest);

    std::string _output_filename;
    rapidjson::Document _value;
    std::vector<Message> _messages; // 包括 args type 和 reply type
    std::map<std::string, std::vector<RPCMethod>> _methods; // service name -> rpcmethod
};

}
}

#endif
