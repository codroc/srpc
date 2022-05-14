#include "acceptor.h"

#include <memory>


Acceptor::Acceptor(EventLoop* loop, TCPSocket::ptr socket, Address addr) 
    : _loop(loop)
    , _sock(socket)
    , _channel(std::make_shared<Channel>(_loop, _sock->fd()))
{
    _channel->set_read_callback(std::bind(&Acceptor::handle_read, this));
    _sock->bind(addr);
}

void Acceptor::handle_read() {
    if (_cb) _cb();
}
