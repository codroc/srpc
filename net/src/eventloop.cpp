#include "eventloop.h"
#include "channel.h"
#include "flog.h"

#include <chrono>
#include <vector>

thread_local EventLoop* t_eventloop = nullptr;

EventLoop::EventLoop()
    : _looping(false)
    , _reactor()
    , _thread_id(std::this_thread::get_id())
{
    if (t_eventloop) {
        LOG_ERROR << "Must be one loop per thread!\n";
        exit(-1);
    } else {
        t_eventloop = this;
    }
}

EventLoop::~EventLoop() {
    assert(!_looping);
    t_eventloop = nullptr;
}

void EventLoop::loop() {
    assert(!_looping);
    assert_in_loop_thread();
    _looping = true;

    // 用于测试 evl_0 1 2
    // std::chrono::seconds(5);
    while (1) {
        // 不断轮询
        _channels.clear();
        _reactor.poll(_channels);        
        for (auto channel : _channels) {
            channel->handle_event();
        }
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
