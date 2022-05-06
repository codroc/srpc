#include "eventloop.h"
#include "flog.h"
#include <chrono>

thread_local EventLoop* t_eventloop = nullptr;

EventLoop::EventLoop()
    : _looping(false)
    , _poll()
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

    std::chrono::seconds(5);

    _looping = false;
}

void EventLoop::abort_thread() {
    LOG_ERROR << "Not in loop thread! Abort thread now!\n";
    exit(-1);
}
