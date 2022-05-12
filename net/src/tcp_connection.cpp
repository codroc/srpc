#include "flog.h"
#include "tcp_connection.h"
#include "eventloop.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>
#include <cassert>

TCPConnection::TCPConnection(EventLoop* loop, const TCPSocket::ptr& socket)
    : _loop(loop)
    , _sock(socket)
    , _channel(std::make_shared<Channel>(loop, _sock->fd()))
    , _in()
    , _out()
    , _status(Status::kConnecting)
{
    _channel->set_read_callback(std::bind(&TCPConnection::handle_read, this));
    _channel->set_write_callback(std::bind(&TCPConnection::handle_write, this));
    _channel->monitor_read(true);
}

void TCPConnection::handle_read() {
    // std::string str = _sock->recv();
    std::string str;
    ssize_t n = _sock->recv(str);
    if (n < 0) { // fix me: string::size 是不可能返回 n < 0 这种情况的。这里怎么办？
        std::cout << "read error! str error: "
                  << ::strerror(errno) << "\n";
        exit(-1);
    } else if (n == 0) {
        assert(_status == Status::kConnected);
        if (_connection_callback) _connection_callback(shared_from_this());
        if (_status == Status::kConnected) {
            _channel->monitor_nothing();
            _status = Status::kClosed;
        }
    } else {
        _in.append(str);
        if (_message_callback) _message_callback(shared_from_this(), _in);
    }
}

void TCPConnection::handle_write() {
    // 触发 IO 可写事件
    // 1. 如果 out bound buffer 中没有数据，那么停止观察 IO 可写事件
    // 2. 如果 out bound buffer 中有数据 peek 出所有数据 msg，调用 Socket::send，得到返回值 n
    //      2.1. 如果 n == msg.size()， 清空 buffer 并停止观察 IO 可写事件
    //      2.2. 如果 n < msg.size()，更新 buffer，且继续观察 IO 可写事件
    if (_out.empty()) {
        _channel->monitor_write(false);
    } else {

    }
}

size_t TCPConnection::send(const std::string& msg) {
    if (_out.empty()) {
        ssize_t n = _sock->send(msg);
        if (n < 0) {
            if (errno == EAGAIN or errno == EWOULDBLOCK or errno == EINTR) {
                n = 0;
            } else {
                LOG_ERROR << "TCPConnection::send Socket::send error! "
                     << ::strerror(errno);
                ::exit(-1);
            }
        }

        if (n < msg.size()) {
            _out.append(msg.substr(n, msg.size() - n));
            _channel->monitor_write(true);
        }
    } else {
        assert(_channel->get_events() & Channel::kWriteEvent);
        _out.append(msg);
    }
    return 0;
}

size_t TCPConnection::send(const char* msg, int len) {
    return send({msg, static_cast<std::string::size_type>(len)});
}

// c-style string (terminate with '\0')
size_t TCPConnection::send(const char* msg) {
    return send(msg, ::strlen(msg));
}
