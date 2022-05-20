#ifndef SRPC_NET_TIMER_H
#define SRPC_NET_TIMER_H

#include <memory>
#include <functional>
#include <atomic>
#include <chrono>

class TimerId {
public:
    TimerId()
        : _id(0)
        , _valid(false)
    {}

    TimerId(uint64_t id)
        : _id(id)
        , _valid(true)
    {}

    uint64_t raw() const { return _id; }
    bool operator<(const TimerId& lhs) { return _id < lhs._id; }
private:
    uint64_t _id;
    bool _valid;
};

class Timer {
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;
    using ptr = std::shared_ptr<Timer>;
    using TimerCallback = std::function<void()>;
    // brief:
    // param interval: double 接口默认单位是秒
    Timer(TimerCallback cb, TimePoint when, double interval);

    // brief: 秒级定时器
    // param interval: std::chrono::duration represents a time interval
    Timer(TimerCallback cb, TimePoint when, std::chrono::seconds interval);

    // brief: 毫秒级定时器
    // param interval: std::chrono::duration represents a time interval
    Timer(TimerCallback cb, TimePoint when, std::chrono::milliseconds interval);

    // brief: 微秒级定时器
    // param interval: std::chrono::duration represents a time interval
    Timer(TimerCallback cb, TimePoint when, std::chrono::microseconds interval);

    void run() const { _timer_callback(); }

    TimePoint expiration() const { return _expiration; }

    bool repeat() const { return _repeat; }

    std::chrono::microseconds interval() const { return _interval; }

    TimerId sequence() const { return _sequence; }

    void restart(TimePoint now);
    void restart(TimerCallback cb, TimePoint now);
public:
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    static uint64_t NumberOfTimer() { return s_timers.load(); }
private:
    // brief: 定时到期的回调函数
    TimerCallback _timer_callback;
    
    // brief: 到期时间
    TimePoint _expiration;

    // brief: 是否周期性定时
    const bool _repeat;

    // brief: 如果是周期性定时，那么间隔时间是多少微秒
    const std::chrono::microseconds _interval;

    // brief: 目前系统中有多少个定时器
    static std::atomic<uint64_t> s_timers; 

    // brief: 该定时器的序号
    const TimerId _sequence;

    // brief: 定时器是否有效
    bool _valid;
};

#endif
