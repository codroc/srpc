#include <iostream>
#include "timer.h"
#include "eventloop.h"
using namespace std;
using namespace std::chrono_literals;

const long long kNanoSecondsPerSecond = 1000000000;
const long long kNanoSecondsPerMilliSecond = 1000000;
const long long kNanoSecondsPerMicroSecond = 1000;

int main() {
    EventLoop loop;
    loop.run_every(2s, [](){
            long long ticks = (long long) chrono::steady_clock::now().time_since_epoch().count();
            cout << "msg " << ticks / kNanoSecondsPerMilliSecond << "."
                 << ticks % kNanoSecondsPerMilliSecond << " every 2 ms\n";
            });
    loop.loop();
    return 0;
}

