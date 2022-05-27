#include <sys/syscall.h>
#include <unistd.h>
#include <thread>
#include "eventloop.h"
using namespace std;

void func(EventLoop* loop) {
    pid_t tid = ::syscall(SYS_gettid);
    loop->run_every(1, [tid]{ printf("thread: %ld say hello.\n", tid); });
    while (1) {}
}

int main() {
    EventLoop loop;
    loop.run_every(2, []() { printf("main thread: %ld say hello.\n", ::syscall(SYS_gettid)); });
    thread t(bind(func, &loop));
    loop.loop();
    t.join();
    return 0;
}
