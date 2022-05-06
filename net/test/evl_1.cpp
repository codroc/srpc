// test one loop per thread, and this is a negative example
#include <iostream>
#include <thread>
#include "eventloop.h"
using namespace std;

EventLoop* g_loop = nullptr;

void func() {
    g_loop->loop();
}

int main() {
    EventLoop loop1;
    g_loop = &loop1;
    
    thread t(func);
    t.join();
    return 0;
}
