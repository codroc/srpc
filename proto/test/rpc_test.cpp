#include "helloworld.h"

#include "address.h"

#include "flog.h"

#include <iostream>
#include <string>
#include <map>
#include <memory>

void test_chrone() {
    std::cout << "size of RPCHeader: " << sizeof(srpc::rpc::RPCHeader) << std::endl;
    std::cout << "size of map: " << srpc::rpc::GetInstanceMap().size() << std::endl;
}

void test_client_codeco() {
    LOG_INFO << "test_client_codeco" << "\n";
    Address addr("127.0.0.1", 8989);
    Greeter::Stub stub(addr);
    SayHelloArgs args;
    std::string name;
    std::cout << "please input name:\n";
    std::cin >> name;
    args.name = name;
    SayHelloReply reply;
    auto ok = stub.SayHello(&args, &reply);
    if (ok.ok()) {
        std::cout << "reply: " << reply.reply << std::endl;
    } else {
        std::cout << "error !\n";
    }
}

int main() {
    test_client_codeco();
    return 0;
}
