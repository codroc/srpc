#ifndef SRPC_NET_TCP_CONNECTION_H
#define SRPC_NET_TCP_CONNECTION_H

#include "buffer.h"
#include "socket.h"
#include "channel.h"
#include <memory>
#include <string>
#include <functional>

// TCPConnection 表示一次 TCP 连接，它是不可再生的，一旦连接断开，这个对象就毫无意义了
// TCPConnection 没有发起连接的功能，它默认情况下就是 已经建立好连接了，因此默认状态是 kConnecting
class EventLoop;
class TCPSocket;
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
    using ptr = std::shared_ptr<TCPConnection>;
    using MessageCallback = std::function<void(TCPConnection::ptr, Buffer&)>;
    using ConnectionCallback = std::function<void(TCPConnection::ptr)>;
    enum class Status {
        kConnecting,
        kConnected,
        kClosed,
    };
    TCPConnection(EventLoop* loop, const TCPSocket::ptr& socket);

    void handle_read();
    void handle_write();

    size_t send(const std::string& msg);
    size_t send(const char* msg, int len);

    // c-style string (terminate with '\0')
    size_t send(const char* msg);

    void set_message_callback(MessageCallback cb) { _message_callback = cb; }
    void set_connection_callback(ConnectionCallback cb) { _connection_callback = cb; }

    Address get_peer_address() const { return _sock->get_peer_address(); }
    Address get_local_address() const { return _sock->get_peer_address(); }

    // return fd
    int fd() const { return _sock->fd(); }
public:
    // 不允许被拷贝
    TCPConnection(const TCPConnection&) = delete;
    TCPConnection& operator=(const TCPConnection&) = delete;
private:
    EventLoop* _loop{};
    TCPSocket::ptr _sock;
    Channel::ptr _channel;

    // 入境字节流缓冲区
    Buffer _in;

    // 出境字节流缓冲区
    Buffer _out;

    Status _status;

    // callback
    MessageCallback _message_callback;
    ConnectionCallback _connection_callback;
};

#endif
