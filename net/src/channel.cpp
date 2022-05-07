#include "channel.h"
#include "eventloop.h"

// EPOLL EVENT see [man 2 epoll_ctl]
const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI; // see [man 2 poll] POLLPRI
const uint32_t Channel::kWriteEvent= EPOLLOUT;

Channel::Channel(EventLoop* loop, FDescriptor fd) 
    : _loop(loop)
    , _fd(fd)
    , _read_callback()
    , _write_callback()
    , _error_callback()
{}

void Channel::handle_event() const {
    if (get_revents() | kReadEvent and _read_callback) {
        _read_callback();
    } else if (get_revents() | kWriteEvent and _write_callback) {
        _write_callback();
    }
}

void Channel::updata() { _loop->updata_channel(this); }
