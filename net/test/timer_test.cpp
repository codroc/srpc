#include <iostream>
#include "timer.h"
using namespace std;

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
    return 0;
}
