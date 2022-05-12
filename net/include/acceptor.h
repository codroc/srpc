#ifndef SRPC_NET_ACCEPTOR_H
#define SRPC_NET_ACCEPTOR_H

#include "socket.h"
#include "channel.h"
#include <functional>

class EventLoop;
class Acceptor {
public:
    using Callback = std::function<void()>;
    Acceptor(EventLoop* loop, TCPSocket::ptr socket);
    void handle_read();

    void listen() {
        _sock->listen();
        _channel->monitor_read(true);
    }

    void set_callback(Callback cb) { _cb = cb; }
private:
    EventLoop* _loop;
    TCPSocket::ptr _sock;
    Channel::ptr _channel;

    Callback _cb;
};

#endif
