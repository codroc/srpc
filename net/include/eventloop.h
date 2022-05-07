#ifndef SRPC_NET_EVENTLOOP_H
#define SRPC_NET_EVENTLOOP_H

#include "poll.h"
#include <thread>

class Channel;
class EventLoop {
public:
    using PollType = Epoll;
    using ActivateChannels = std::vector<Channel*>; // fix me: 这里 Channel* 会不会变成空悬指针？
    EventLoop();
    ~EventLoop();

    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            abort_thread();
        }
    }

    bool is_in_loop_thread() { return _thread_id == std::this_thread::get_id(); }

    void loop();

    // 注册/更新 Channel
    void updata_channel(Channel* channel);

    // 删除 copy 函数
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
private:
    void abort_thread();
private:
    bool _looping{};
    PollType _reactor;
    std::thread::id _thread_id;

    // 有事件发生的 IO socket
    ActivateChannels _channels;
};

#endif
