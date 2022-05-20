#include "eventloop.h"
#include "channel.h"
#include "flog.h"

#include <sys/eventfd.h>

#include <chrono>
#include <vector>
#include <iostream>

thread_local EventLoop* t_eventloop = nullptr;

using namespace std::chrono_literals;

EventLoop::EventLoop()
    : _mutex()
    , _looping(false)
    , _reactor()
    , _thread_id(std::this_thread::get_id())
    , _event_channel(this, ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
    , _tasks()
    , _timer_channel(this)
    , _timers()
{
    if (t_eventloop) {
        LOG_ERROR << "Must be one loop per thread!\n";
        exit(-1);
    } else {
        t_eventloop = this;
    }

    _event_channel.set_read_callback(std::bind(&EventLoop::wait, this));
    _event_channel.monitor_read(true);

    _timer_channel.set_timeout_callback(std::bind(&EventLoop::on_timeout, this));
}

EventLoop::~EventLoop() {
    assert(!_looping);
    t_eventloop = nullptr;
}

void EventLoop::loop(int timeout) {
    assert(!_looping);
    assert_in_loop_thread();
    _looping = true;

    // 用于测试 evl_0 1 2
    // std::chrono::seconds(5);
    while (1) {
        // 不断轮询
        _channels.clear();
        _reactor.poll(_channels, timeout);
        for (auto channel : _channels) {
            channel->handle_event();
        }

        run_task();
    }

    _looping = false;
}

void EventLoop::abort_thread() {
    LOG_ERROR << "Not in loop thread! Abort thread now!\n";
    exit(-1);
}

void EventLoop::updata_channel(Channel* channel) {
    if (channel->is_added_to_reactor()) {
        if (channel->get_events() == Channel::kNoneEvent) {// 删除
            _reactor.unenroll(channel, channel->get_events());
            channel->add_to_reactor(false);
        }
        else {// 更新
            _reactor.updata(channel, channel->get_events());
        }
    } else {
        if (channel->get_events() != Channel::kNoneEvent) {
            // 注册
            _reactor.enroll(channel, channel->get_events());
            channel->add_to_reactor(true);
        }
    }
}

void EventLoop::run_task() {
    std::vector<Task> tasks;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        tasks.swap(_tasks);
    }
    for (Task t : tasks)
        t();
}

void EventLoop::run_in_loop(EventLoop::Task t) {
    std::lock_guard<std::mutex> guard(_mutex);
    _tasks.push_back(t);
    wakeup();
}

void EventLoop::wait() {
    uint64_t val;
    ::read(_event_channel.fd(), &val, sizeof val);
}

void EventLoop::wakeup() {
    uint64_t val = 1;
    ::write(_event_channel.fd(), &val, sizeof val);
}

void EventLoop::insert_timer_queue(Timer::ptr timer) {
    std::lock_guard<std::mutex> guard(_mutex);
    while (!_timers.empty() and _timer_ids.find(_timers.top()->sequence()) == _timer_ids.end())
        _timers.pop();

    _timer_ids.insert(timer->sequence());
    _timers.push(timer);
    _timer_channel.update_timerfd_expiration(_timers.top()->expiration());
}

void EventLoop::cancel_timer(TimerId id) {
    _timer_ids.erase(id);
}

bool EventLoop::cancelled(Timer::ptr sp_timer) const {
    return _timer_ids.find(sp_timer->sequence()) == _timer_ids.end();
}

void EventLoop::on_timeout() {
    auto now = std::chrono::steady_clock::now();
    std::vector<Timer::ptr> timers;
    while (!_timers.empty() and _timers.top()->expiration() <= now) {
        auto sp_timer = _timers.top();
        _timers.pop();
        if (!cancelled(sp_timer)) {
            timers.push_back(sp_timer);
            cancel_timer(sp_timer->sequence());
        }
    }
    for (auto elem : timers)
        elem->run();
    for (auto elem : timers) {
        if (elem->repeat()) {
            elem->restart(std::chrono::steady_clock::now());
            insert_timer_queue(elem);
        }
    }
    // if (!_timers.empty())
    //     _timer_channel.update_timerfd_expiration(_timers.top()->expiration());
}

TimerId EventLoop::run_at(Timer::TimePoint when, Timer::TimerCallback cb) {
    if (when < std::chrono::steady_clock::now())
        return {};
    Timer::ptr timer = std::make_shared<Timer>(cb, when, 0);
    insert_timer_queue(timer);
    return timer->sequence();
}

TimerId EventLoop::run_after(double duration, Timer::TimerCallback cb) {
    return run_after(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::duration<double>(duration)
                ), cb);
}

TimerId EventLoop::run_after(std::chrono::seconds duration, Timer::TimerCallback cb) {
    return run_after(
            std::chrono::duration_cast<std::chrono::microseconds>(duration), cb);
}

TimerId EventLoop::run_after(std::chrono::milliseconds duration, Timer::TimerCallback cb) {
    return run_after(
            std::chrono::duration_cast<std::chrono::microseconds>(duration), cb);
}

TimerId EventLoop::run_after(std::chrono::microseconds duration, Timer::TimerCallback cb) {
    return run_at(std::chrono::steady_clock::now() + duration, cb);
}

TimerId EventLoop::run_every(double duration, Timer::TimerCallback cb) {
    return run_every(std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::duration<double>(duration)), cb);
}

TimerId EventLoop::run_every(std::chrono::seconds duration, Timer::TimerCallback cb) {
    return run_every(std::chrono::duration_cast<std::chrono::microseconds>(
                duration), cb);
}

TimerId EventLoop::run_every(std::chrono::milliseconds duration, Timer::TimerCallback cb) {
    return run_every(std::chrono::duration_cast<std::chrono::microseconds>(
                duration), cb);
}

TimerId EventLoop::run_every(std::chrono::microseconds duration, Timer::TimerCallback cb) {
    auto when = std::chrono::steady_clock::now() + duration;
    Timer::ptr timer = std::make_shared<Timer>(cb, when, duration);
    insert_timer_queue(timer);
    return timer->sequence();
}

