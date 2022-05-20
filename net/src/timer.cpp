#include "timer.h"

std::atomic<uint64_t> Timer::s_timers{0};

Timer::Timer(TimerCallback cb, TimePoint when, std::chrono::microseconds interval)
    : _timer_callback(cb)
    , _expiration(when)
    , _repeat(interval > std::chrono::microseconds(0))
    , _interval(interval)
    , _sequence(s_timers.fetch_add(1, std::memory_order_relaxed))
    , _valid(true)
{}

Timer::Timer(TimerCallback cb, TimePoint when, std::chrono::seconds interval)
    : Timer(cb, when, std::chrono::microseconds(interval))
{}

Timer::Timer(TimerCallback cb, TimePoint when, std::chrono::milliseconds interval)
    : Timer(cb, when, std::chrono::microseconds(interval))
{}

Timer::Timer(TimerCallback cb, TimePoint when, double interval)
    : Timer(cb, when, 
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(interval)))
{}

void Timer::restart(TimerCallback cb, TimePoint now) {
    _timer_callback = cb;
    restart(now);
}

void Timer::restart(TimePoint now) {
    if (!_repeat)
        _valid = false;
    else
        _expiration = now + _interval;
}
