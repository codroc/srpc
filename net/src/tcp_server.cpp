#include "eventloop.h"
#include "tcp_server.h"
#include <iostream>
#include <memory>

TCPServer::TCPServer(EventLoop* loop, Address addr, SocketOptions opts) 
    : _loop(loop)
    , _acceptor(_loop, std::make_shared<TCPSocket>(opts), addr)
    , _conns()
{
    _acceptor.set_callback(std::bind(&TCPServer::handle_accept, this));
}

TCPServer::~TCPServer() {
    _conns.clear();
}

void TCPServer::start() {
    _acceptor.listen();
}

void TCPServer::OnClose(TCPConnection::ptr conn) {
    _conns.erase(conn->fd());
    _loop->run_in_loop(std::bind(&TCPConnection::connect_destroyed, conn));
}

void TCPServer::handle_accept() {
    TCPConnection::ptr conn = std::make_shared<TCPConnection>(_loop, std::make_shared<TCPSocket>(_acceptor.accept()));
    int fd = conn->fd();
    if (fd >= 0) {
        // std::cout << "new connection from [" << conn->get_peer_address().ip()
        //      << ":" << conn->get_peer_address().port()
        //      << "]\n";

        conn->set_status(TCPConnection::Status::kConnected);
        conn->set_message_callback(std::bind(&TCPServer::OnMessage, this, std::placeholders::_1, 
                    std::placeholders::_2));
        conn->set_close_callback(std::bind(&TCPServer::OnClose, this, std::placeholders::_1));

        _conns.insert({fd, conn});
    } else {
        std::cout << "accept error!\n";
        exit(-1);
    }
}
