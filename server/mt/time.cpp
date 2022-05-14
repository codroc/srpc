// 这是一个用于测试 ab 压测能否正常使用在 net 模块上的程序
// 由于 ab 仅支持 HTTP1.0 因此在建立起连接后，接受对端一个数据报，然后服务器发回一个 当前时间，立马断开连接
#include "net.h"
#include "acceptor.h"
#include "tcp_server.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <map>
#include <iostream>
#include <string>
#include <memory>
#include <chrono>

class TimeServer : public TCPServer {
public:
    TimeServer(EventLoop* loop, Address addr, SocketOptions opts = SocketOptions())
        : TCPServer(loop, addr, opts)
    {}

    virtual void OnConnection(TCPConnection::ptr conn) override {
    };
    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) override {
        auto t = std::chrono::system_clock::now();
        std::time_t today_time = std::chrono::system_clock::to_time_t(t);
        conn->send(std::ctime(&today_time));
        conn->force_close();
    };
};

int main() {
    SocketOptions opts;
    opts.blocking = false; // 非阻塞
    opts.reuseaddr = true;

    EventLoop loop;

    TimeServer serv(&loop, {"127.0.0.1", 8080}, opts);
    serv.start();

    loop.loop();
    return 0;
}
