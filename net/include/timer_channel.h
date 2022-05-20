#ifndef SRPC_NET_TIMER_CHANNEL_H
#define SRPC_NET_TIMER_CHANNEL_H
#include <sys/timerfd.h>
#include "channel.h"
#include "timer.h"

#include <functional>

class EventLoop;
class TimerChannel {
public:
    using TimeoutCallback = std::function<void()>;
    TimerChannel(EventLoop* loop);

    ~TimerChannel();

    // 更新 timerfd 的 expiration 时间
    // struct timespec {
    //     time_t tv_sec;                /* Seconds */
    //     long   tv_nsec;               /* Nanoseconds */
    // };

    // struct itimerspec {
    //     struct timespec it_interval;  /* Interval for periodic timer */
    //     struct timespec it_value;     /* Initial expiration */
    // };
    void update_timerfd_expiration(Timer::TimePoint new_expiration);

    void set_timeout_callback(TimeoutCallback cb) { _timeout_callback = cb; }
private:
    void handle_read();
    struct timespec how_much_from_now(Timer::TimePoint new_expiration);
private:
    EventLoop* _loop;
    Channel _channel;
    TimeoutCallback _timeout_callback;
};

#endif
