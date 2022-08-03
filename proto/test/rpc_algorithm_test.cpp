#include "algorithm.srpc.h"

#include "address.h"

#include <iostream>
#include <string>
#include <map>
#include <memory>

int main() {
    Address addr("127.0.0.1", 6677);
    Cal::Stub stub(addr);
    AddArgs args;
    int32_t a, b;
    std::cout << "please input a and b:\n";
    std::cin >> a >> b;
    args.a = a;
    args.b = b;

    AddReply reply;
    auto ok = stub.Add(&args, &reply);

    if (ok.ok()) {
        std::cout << "reply: " << reply.res << std::endl;
    } else {
        std::cout << "error !\n";
    }

    return 0;
}


