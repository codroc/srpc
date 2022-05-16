#include "flog.h"
#include "net.h"
#include <iostream>
#include <atomic>

using namespace std;

atomic<int> g_total_conns{0};

class EchoServer : public TCPServer {
public:
    EchoServer(EventLoop* loop, Address addr, SocketOptions opts)
        : TCPServer(loop, addr, opts)
    {}

    virtual void OnConnection(TCPConnection::ptr conn) override;
    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) override;
};

void EchoServer::OnConnection(TCPConnection::ptr conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection from ["
            << conn->get_peer_address().ip()
            << ":" << conn->get_peer_address().port()
            << "] id = " << g_total_conns << "\n";
    } else {
        LOG_INFO << "Connection from ["
            << conn->get_peer_address().ip()
            << ":" << conn->get_peer_address().port()
            << "] is closed!\n";
    }
}

void EchoServer::OnMessage(TCPConnection::ptr conn, Buffer& buffer) {
    g_total_conns++;
    std::string msg1(1000, 'c');
    std::string msg2(msg1);
    std::string msg3(msg1);
    std::string msg4(msg1);
    std::string msg5(msg1);
    conn->send(msg5);
    conn->force_close();
    // cout << g_total_conns << endl;
}

AsyncLogInit mylog("/tmp/log");
int main() {

    SocketOptions opts;
    opts.blocking = false;
    opts.reuseaddr = true;

    EventLoop loop;
    EchoServer serv(&loop, {"127.0.0.1", 8080}, opts);
    serv.start();

    loop.loop();

    // log.destroy();
    return 0;
}
