// 这是一个用于测试 ab 压测能否正常使用在 net 模块上的程序
// 由于 ab 仅支持 HTTP1.0 因此在建立起连接后，接受对端一个数据报，然后服务器发回一个 当前时间，立马断开连接
#include "net.h"
#include "acceptor.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <map>
#include <iostream>
#include <string>
#include <memory>
#include <chrono>

using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

std::map<int, TCPConnectionPtr> g_conns;

void OnMessage(TCPConnectionPtr conn, Buffer& buffer) {
    auto t = std::chrono::system_clock::now();
    std::time_t today_time = std::chrono::system_clock::to_time_t(t);
    conn->send(std::ctime(&today_time));
    g_conns.erase(conn->fd());
}

void handle_accept(EventLoop* loop, TCPSocket* serv) {
    TCPConnectionPtr conn = std::make_shared<TCPConnection>(loop, std::make_shared<TCPSocket>(serv->accept()));
    int fd = conn->fd();
    if (fd >= 0) {
        std::cout << "new connection from [" << conn->get_peer_address().ip()
             << ":" << conn->get_peer_address().port()
             << "]\n";

        conn->set_message_callback(std::bind(OnMessage, std::placeholders::_1, 
                    std::placeholders::_2));

        g_conns.insert({fd, conn});
    } else {
        std::cout << "accept error!\n";
        exit(-1);
    }
}

int main() {
    SocketOptions opts;
    opts.blocking = false; // 非阻塞
    opts.reuseaddr = true;

    TCPSocket serv(opts);
    serv.bind({"127.0.0.1", 8080});

    EventLoop loop;
    auto sp = std::make_shared<TCPSocket>(serv);
    Acceptor a(&loop, sp);
    a.set_callback(std::bind(handle_accept, &loop, sp.get()));
    a.listen();

    loop.loop();

    return 0;
}
