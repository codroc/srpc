#ifndef SRPC_NET_EVENTLOOP_H
#define SRPC_NET_EVENTLOOP_H

#include "poll.h"
#include <thread>

class EventLoop {
public:
    using PollType = Epoll;
    EventLoop();
    ~EventLoop();

    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            abort_thread();
        }
    }

    bool is_in_loop_thread() { return _thread_id == std::this_thread::get_id(); }
    void loop();

    // 删除 copy 函数
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
private:
    void abort_thread();
private:
    bool _looping{};
    PollType _poll;
    std::thread::id _thread_id;
};

#endif
