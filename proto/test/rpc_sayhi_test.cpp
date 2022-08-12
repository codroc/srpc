#include "sayhi.srpc.h"

#include "address.h"

#include <iostream>
#include <string>
#include <map>
#include <memory>

int main() {
    Address addr("124.70.82.205", 12345);
    Hi::Stub stub(addr);
    SayHiArgs args;
    std::string name;
    std::cout << "please input name:\n";
    std::cin >> name;
    args.name = name;

    SayHiReply reply;
    auto ok = stub.SayHi(&args, &reply);

    if (ok.ok()) {
        std::cout << "reply: " << reply.reply << std::endl;
    } else {
        std::cout << "error !\n";
    }

    return 0;
}

