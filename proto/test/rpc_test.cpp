#include "rpc.h"
#include "address.h"
#include "codeco.h"
#include "message.h"
#include "serialize.h"
#include "helloworld.h"
#include <iostream>
#include <string>
#include <map>
#include <memory>

void test_chrone() {
    std::cout << "size of RPCHeader: " << sizeof(srpc::rpc::RPCHeader) << std::endl;
    std::cout << "size of map: " << srpc::rpc::GetInstanceMap().size() << std::endl;
}

void test_client_codeco() {
    Address addr("127.0.0.1", 8989);
    Greeter::Stub stub(addr);
    SayHelloArgs args;
    args.name = "codroc";
    SayHelloReply reply;
    auto ok = stub.SayHello(&args, &reply);
    if (ok.ok()) {
        std::cout << "reply: " << reply.reply << ", should same as: " << args.name << std::endl;
    } else {
        std::cout << "error !\n";
    }
}

int main() {
    test_client_codeco();
    return 0;
}
