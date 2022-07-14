#include "rpc.h"
#include "codeco.h"
#include "message.h"
#include "serialize.h"
#include "helloworld.h"
#include <iostream>
#include <string>
#include <map>
#include <memory>

int main() {
    std::cout << "size of map: " << srpc::rpc::GetInstanceMap().size() << std::endl;
    return 0;
}
