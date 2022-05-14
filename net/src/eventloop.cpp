#include "eventloop.h"
#include "channel.h"
#include "flog.h"

#include <sys/eventfd.h>

#include <chrono>
#include <vector>
#include <iostream>

thread_local EventLoop* t_eventloop = nullptr;

EventLoop::EventLoop()
    : _mutex()
    , _looping(false)
    , _reactor()
    , _thread_id(std::this_thread::get_id())
    , _event_channel(this, ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
    , _tasks()
{
    if (t_eventloop) {
        LOG_ERROR << "Must be one loop per thread!\n";
        exit(-1);
    } else {
        t_eventloop = this;
    }

    _event_channel.set_read_callback(std::bind(&EventLoop::wait, this));
    _event_channel.monitor_read(true);
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
