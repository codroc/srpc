#ifndef SRPC_NET_EVENTLOOP_H
#define SRPC_NET_EVENTLOOP_H

#include "poll.h"
#include "channel.h"

#include <unistd.h>

#include <thread>
#include <functional>
#include <mutex>

class EventLoop {
public:
    using PollType = Epoll;
    using ActivateChannels = std::vector<Channel*>;
    using Task = std::function<void()>;
    using Tasks = std::vector<Task>;
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
};

#endif
