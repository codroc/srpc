#include <sys/syscall.h>
#include <unistd.h>
#include <thread>
#include "eventloop.h"
using namespace std;

void func(EventLoop* loop) {
    loop->run_every(1, []{ printf("func say hello.\n"); });
    while (1) {}
}

int main() {
    EventLoop loop;
    loop.run_every(2, []() { printf("main say hello.\n"); });
    thread t(bind(func, &loop));
    loop.loop();
    t.join();
    return 0;
}
