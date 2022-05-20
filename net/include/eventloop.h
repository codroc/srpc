#ifndef SRPC_NET_EVENTLOOP_H
#define SRPC_NET_EVENTLOOP_H

#include "poll.h"
#include "channel.h"
#include "timer.h"
#include "timer_channel.h"

#include <unistd.h>

#include <unordered_set>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>

struct TimerCmp {
    bool operator()(const Timer::ptr& rhs, const Timer::ptr& lhs) const {
        return rhs->expiration() > lhs->expiration(); 
    }
};

struct TimerIdHash {
    size_t operator()(const TimerId& rhs) const {
        return std::hash<uint64_t>()(rhs.raw());
    }
};

struct TimerIdCmp {
    bool operator()(const TimerId& lhs, const TimerId& rhs) const {
        return rhs.raw() == lhs.raw();
    }
};

class EventLoop {
public:
    using PollType = Epoll;
    using ActivateChannels = std::vector<Channel*>;
    using Task = std::function<void()>;
    using Tasks = std::vector<Task>;
    using TimerIds = std::unordered_set<TimerId, TimerIdHash, TimerIdCmp>;
    using Timers = std::priority_queue<Timer::ptr, std::vector<Timer::ptr>, TimerCmp>;
    EventLoop();
    ~EventLoop();

    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            abort_thread();
        }
    }

    bool is_in_loop_thread() { return _thread_id == std::this_thread::get_id(); }

    void loop(int timeout = -1);

    // 注册/更新 Channel
    void updata_channel(Channel* channel);

    // brief: 其他线程放任务进来，非线程安全
    void run_in_loop(Task t);

    void run_task();

    // 定时器相关
    TimerId run_at(Timer::TimePoint when, Timer::TimerCallback cb);

    // brief: 默认是 秒
    TimerId run_after(double duration, Timer::TimerCallback cb);
    // std::chrono 的 duration 接口
    TimerId run_after(std::chrono::seconds duration, Timer::TimerCallback cb);
    TimerId run_after(std::chrono::milliseconds duration, Timer::TimerCallback cb);
    TimerId run_after(std::chrono::microseconds duration, Timer::TimerCallback cb);

    TimerId run_every(double duration, Timer::TimerCallback cb);
    TimerId run_every(std::chrono::seconds duration, Timer::TimerCallback cb);
    TimerId run_every(std::chrono::milliseconds duration, Timer::TimerCallback cb);
    TimerId run_every(std::chrono::microseconds duration, Timer::TimerCallback cb);

    void cancel_timer(TimerId id);
public:
    // 删除 copy 函数
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
private:
    void abort_thread();
private:
    std::mutex _mutex;
    bool _looping{};
    PollType _reactor;
    std::thread::id _thread_id;

    // 有事件发生的 IO socket
    ActivateChannels _channels;
    Tasks _tasks;

private:
    void wait();

    // brief: 唤醒 epoll_wait 阻塞住的 IO 线程
    // 这里使用 eventfd 的方式，因为可以非常自然地融入 epoll_wait
    void wakeup();

    // eventfd to impletement wait/notify
    Channel _event_channel;

private:
    void on_timeout();
    bool cancelled(Timer::ptr sp_timer) const;
    void insert_timer_queue(Timer::ptr timer);
private:
    TimerChannel _timer_channel;
    TimerIds _timer_ids;
    Timers _timers;
};

#endif
