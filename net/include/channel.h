#ifndef SRPC_NET_CHANNEL_H
#define SRPC_NET_CHANNEL_H

#include "file_descriptor.h"
#include <functional>
#include <memory>

// Channel 不可以被拷贝！因为在析构时需要执行 unregister 操作！
class EventLoop;
class Channel {
public:
    static const uint32_t kNoneEvent;
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;
public:
    using Callback = std::function<void()>;
    using ptr = std::shared_ptr<Channel>;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    
    // 返回一个 封装过的 fd，具体看 file_descriptor.h
    int fd() const { return _fd; }
    
    // 让 eventloop 去调用，然后 channel 根据具体的 IO 事件分发为不同回调
    void handle_event() const;

    void set_read_callback(Callback cb) { _read_callback = cb; }
    void set_write_callback(Callback cb) { _write_callback = cb; }
    void set_error_callback(Callback cb) { _error_callback = cb; }

    // 发生的 IO 事件
    uint32_t get_revents() const { return _revents; }
    void set_revents(uint32_t revents) { _revents = revents; }

    // 观察的 IO 事件
    uint32_t get_events() const { return _events; }
    void set_events(uint32_t events) { _events = events; }

    // 监视 可读 事件
    void monitor_read(bool flag) {
        if (flag)
            _events |= kReadEvent;
        else
            _events &= ~kReadEvent;
        updata();
    }

    // 监视 可写 事件
    void monitor_write(bool flag) {
        if (flag)
            _events |= kWriteEvent;
        else
            _events &= ~kWriteEvent;
        updata();
    }

    // 不监视事件
    void monitor_nothing() {
        _events = kNoneEvent;
        updata();
    }

    // 注册/更新/删除 channel 到 io multiplexing 机制中去
    void updata();

    bool is_added_to_reactor() const { return _added_to_reactor; }
    void add_to_reactor(bool flag) { _added_to_reactor = flag; }

public:
    // Channel() = default;
    // 不允许拷贝
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
private:
    EventLoop* _loop;
    int _fd;

    Callback _read_callback;
    Callback _write_callback;
    Callback _error_callback;

    // 发生的 IO 事件
    uint32_t _revents{};
    
    // 观察的 IO 事件
    uint32_t _events{};

    // 是否以及添加到 reactor 中
    bool _added_to_reactor{};
};

#endif
