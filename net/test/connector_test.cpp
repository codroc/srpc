#include <gtest/gtest.h>
#include "connector.h"
#include <iostream>

int main() {
    EventLoop loop;
    Connector connector(&loop, Address{"127.0.0.1", 8080});
    connector.connect();
    loop.loop();
    return 0;
}

