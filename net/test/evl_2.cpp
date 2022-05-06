// test one loop per thread, and this is a negative example
// test one thread with two eventloop(must be abort!)
#include <iostream>
#include <thread>
#include "eventloop.h"
using namespace std;

void func() {
    EventLoop e1;
    EventLoop e2;
}

int main() {
    thread t(func);
    t.join();
    return 0;
}
