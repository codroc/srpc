#include <iostream>
#include "timer.h"
#include "eventloop.h"
using namespace std;
using namespace std::chrono_literals;

int main() {
    int times = 0;
    auto start = std::chrono::steady_clock::now();
    Timer timer([&]() {
            cout << times++ << endl;
            }, start, 1);
    auto interval = chrono::duration_cast<chrono::microseconds>(timer.interval());
    cout << "Number " << Timer::NumberOfTimer() << " "
         << "timer's interval: " << interval.count() << "us"
         << " ≈ " << interval / 1ms << "ms"
         << " ≈ " << interval / 1s << "s\n";
    auto end = std::chrono::steady_clock::now();
    cout << "Time has gone: " 
         << chrono::duration_cast<chrono::microseconds>(end - start).count()
         << " us\n";

    EventLoop loop;
    const long long kNanoSecondsPerSecond = 1000000000;
    const long long kNanoSecondsPerMilliSecond = 1000000;
    const long long kNanoSecondsPerMicroSecond = 1000;
    loop.run_every(1, [](){ cout << "hello timer 1 !" << chrono::steady_clock::now().time_since_epoch().count() / 1000000000 << "\n"; });
    loop.run_every(100ms, [](){ cout << "hello timer 100ms !" << chrono::steady_clock::now().time_since_epoch().count() / kNanoSecondsPerMilliSecond << "\n"; });
    loop.run_every(2, [](){ cout << "hello timer 2 !" << chrono::steady_clock::now().time_since_epoch().count() / 1000000000 << "\n"; });
    loop.run_every(200ms, [](){ cout << "hello timer 200ms !" << chrono::steady_clock::now().time_since_epoch().count() / kNanoSecondsPerMilliSecond << "\n"; });
    loop.run_every(3, [](){ cout << "hello timer 3 !" << chrono::steady_clock::now().time_since_epoch().count() / 1000000000 << "\n"; });
    loop.loop();
    return 0;
}
