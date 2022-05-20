#include "timer_channel.h"
#include "eventloop.h"
#include "flog.h"
#include <errno.h>
#include <string.h>
// struct timespec {
//     time_t tv_sec;                /* Seconds */
//     long   tv_nsec;               /* Nanoseconds */
// };

// struct itimerspec {
//     struct timespec it_interval;  /* Interval for periodic timer */
//     struct timespec it_value;     /* Initial expiration */
// };
TimerChannel::TimerChannel(EventLoop* loop)
    : _loop(loop)
    , _channel(_loop, ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
    , _timeout_callback()
{
    _channel.set_read_callback(std::bind(&TimerChannel::handle_read, this));
    _channel.monitor_read(true);
}

TimerChannel::~TimerChannel() {
    _channel.monitor_nothing();
    _loop = nullptr;
}

void TimerChannel::handle_read() {
    uint64_t exp = 0;
    if (-1 == ::read(_channel.fd(), &exp, sizeof exp)) {
        LOG_ERROR << "TimerChannel::handle_read error, error string: "
                  << ::strerror(errno) << "\n";
        exit(-1);
    }
    
    if (_timeout_callback) _timeout_callback();
}

struct timespec TimerChannel::how_much_from_now(Timer::TimePoint new_expiration) {
    auto dur = new_expiration - std::chrono::steady_clock::now();
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur) - secs;
    return {secs.count(), ns.count() < 0 ? 1000 : ns.count()};
}

void TimerChannel::update_timerfd_expiration(Timer::TimePoint new_expiration) {
    struct itimerspec new_value{};
    new_value.it_value = how_much_from_now(new_expiration);
    if (-1 == ::timerfd_settime(_channel.fd(), 0, &new_value, 0)) {
        LOG_ERROR << "TimerChannel::update_timerfd_expiration error, error string: "
                  << ::strerror(errno) << "\n"
                  << " fd = " << _channel.fd() << " tv_sec = " << new_value.it_value.tv_sec
                  << " tv_nsec = " << new_value.it_value.tv_nsec << "\n";
        exit(-1);
    }
}


