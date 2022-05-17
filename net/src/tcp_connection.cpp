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
}

TCPConnection::~TCPConnection() {
    assert(_status == Status::kDisconnected);
}

void TCPConnection::force_close() {
    handle_close();
}

void TCPConnection::connect_destroyed() {
    assert(_status == Status::kConnected);
    _status = Status::kDisconnected;
    _channel->monitor_nothing();

    // std::cout << "here\n";
    if (_connection_callback) _connection_callback(shared_from_this());
}

void TCPConnection::handle_close() {
    assert(_status == Status::kConnected);
    // _status = Status::kDisconnected;
    // _channel->monitor_nothing();
    
    // 把 close socket 的动作交给 TCPConnection 的析构函数去，不在这里进行
    TCPConnection::ptr guard(shared_from_this());
    if (_close_callback) _close_callback(shared_from_this());
}

void TCPConnection::handle_error() {
    std::cout << "read error! str error: "
        << ::strerror(errno) << "\n";
    exit(-1);
}

void TCPConnection::handle_read() {
    std::string str;
    ssize_t n = _sock->recv(str);
    if (n < 0) {
        handle_error();
    } else if (n == 0) {
        handle_close();
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
        std::string msg = _out.peek_all();
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

        if (n == msg.size()) {
            _out.reset();
            _channel->monitor_write(false);
        } else {
            _out.pop_out(n);
        }
    }
}

// send_in_loop 只会被 IO 线程调用，因此是安全的
void TCPConnection::send_in_loop(const std::string& msg) {
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
}

// send 可以被用户线程调用，因此不是线程安全的，如何做到线程安全？
// 如果是 非 loop 线程调用了 send，那么就把这个任务丢给 loop 去做
void TCPConnection::send(const std::string& msg) {
    if (!_loop->is_in_loop_thread()) {
        _loop->run_in_loop(std::bind(&TCPConnection::send_in_loop, this, msg));
    } else {
        send_in_loop(msg);
    }
}

void TCPConnection::send(const char* msg, int len) {
    send({msg, static_cast<std::string::size_type>(len)});
}

// c-style string (terminate with '\0')
void TCPConnection::send(const char* msg) {
    send(msg, ::strlen(msg));
}

void TCPConnection::established() {
    assert(_status == Status::kConnecting);
    set_status(Status::kConnected);
    if (_connection_callback) _connection_callback(shared_from_this());
    _channel->monitor_read(true);
}
