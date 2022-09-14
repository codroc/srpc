#ifndef SRPC_NET_CONNECTOR_H
#define SRPC_NET_CONNECTOR_H

#include "eventloop.h"
#include "socket.h"
#include "address.h"
#include "flog.h"
#include "channel.h"
#include <iostream>

class Connector {
public:
    Connector(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions())
        : loop_(loop), addr_(addr), sock_(opts), channel_(loop, sock_.fd())
    {}

    int connect() {
        connectInLoop();
    }

    void handleWrite() {
        std::cout << "Connected to [" << addr_.to_string() << "]\n";
        channel_.monitor_write(false);
    }
private:
    void connectInLoop() {
        loop_->run_in_loop([&]() {
                sock_.connect(addr_);
                channel_.set_write_callback(std::bind(&Connector::handleWrite, this));
                channel_.monitor_write(true);
            });
    }
private:
    EventLoop* loop_;
    Address addr_;
    TCPSocket sock_;
    Channel channel_;
};

#endif
