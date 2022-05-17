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
    using CloseCallback = ConnectionCallback;
    TCPConnection(EventLoop* loop, const TCPSocket::ptr& socket);
    ~TCPConnection();

    enum class Status {
        kConnecting,
        kConnected,
        kDisconnected, // 表示 写端 正在关闭
    };
    // brief: 主动关闭连接，用户可以调用
    void force_close();

    // brief: 判断连接是否建立，可以被用户调用
    bool connected() const { return _status == Status::kConnected; }

    // brief: 3 个 send 都能由用户调用，因此线程不安全
    void send(const std::string& msg);
    void send(const char* msg, int len);

    // c-style string (terminate with '\0')
    void send(const char* msg);

    // brief: 用户调用
    void set_message_callback(MessageCallback cb) { _message_callback = cb; }
    void set_connection_callback(ConnectionCallback cb) { _connection_callback = cb; }

    // brief: 不可以被用户调用
    void set_status(Status s) { _status = s; }

    // brief: 用户不能调用，让 TCPServer 去调用
    void set_close_callback(CloseCallback cb) { _close_callback = cb; }

    Address get_peer_address() const { return _sock->get_peer_address(); }
    Address get_local_address() const { return _sock->get_peer_address(); }

    // brief: return fd，用户不能调用
    int fd() const { return _sock->fd(); }

    // brief: 在 TCPConnection 析构前调用的最后一个方法，用户不能调用，TCPServer 调用
    void connect_destroyed();

    // brief: 当三次握手完成，accept 从 accept queue 中取出连接后，表示连接已建立 established
    void established();
public:
    // 不允许被拷贝
    TCPConnection(const TCPConnection&) = delete;
    TCPConnection& operator=(const TCPConnection&) = delete;
private:
    void handle_read();
    void handle_write();
    void handle_error();
    void handle_close();
    // brief: 只能由 IO 线程调用，线程安全
    void send_in_loop(const std::string& msg);
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
    // 这个回调仅仅供 TCPServer 使用，用于从它的 map 中移除该 TCPConnection
    CloseCallback _close_callback;
};

#endif
