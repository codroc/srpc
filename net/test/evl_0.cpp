// test one loop per thread
#include "eventloop.h"
#include <thread>
#include <iostream>
using namespace std;

void func() {
    cout << "func thread id: " << this_thread::get_id() << endl;
    EventLoop loop;
    loop.loop();
}

int main() {
    cout << "main thread id: " << this_thread::get_id() << endl;
    thread t(func);
    t.join();
    return 0;
}
