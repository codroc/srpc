#ifndef SRPC_NET_TCP_SERVER_H
#define SRPC_NET_TCP_SERVER_H

#include "address.h"
#include "socket.h"
#include "acceptor.h"
#include "buffer.h"
#include "tcp_connection.h"

#include <map>

class EventLoop;
class TCPServer {
public:
    using MapType = std::map<int, TCPConnection::ptr>;
    TCPServer(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions());
    virtual ~TCPServer();

    virtual void OnConnection(TCPConnection::ptr conn) {};
    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) {};

    void start();
private:
    void OnClose(TCPConnection::ptr conn);

    void handle_accept();
private:
    EventLoop* _loop;
    Acceptor _acceptor;
    MapType _conns;
};

#endif
