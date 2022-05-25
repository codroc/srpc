#include "thread_pool.h"
#include "eventloop.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

int main() {
    EventLoop loop;
    ThreadPool pool(2);

    loop.run_after(5s, [&pool]() { 
            cout << "going to stop pool\n";
            pool.stop(); pool.~ThreadPool();});
    loop.loop();
    return 0;
}
