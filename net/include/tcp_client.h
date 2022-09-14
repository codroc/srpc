#ifndef SRPC_NET_TCP_CLIENT_H
#define SRPC_NET_TCP_CLIENT_H

class TCPClient {
public:
    TCPClient(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions())
        : loop_(loop), addr_(addr), connector(loop, addr)
    {}
    void start() { connector_.connect(); }
private:
    EventLoop* loop_;
    Address addr_;
    Connector connector_;
};

#endif
