#include "flog.h"
#include <boost/circular_buffer.hpp>
#include "currentThread.h"
#include "logger.h"
#include "http.h"
#include "net.h"
#include <iostream>
#include <unordered_set>
#include <atomic>

using namespace std;

atomic<int> g_total_conns{0};

class EchoServer : public TCPServer {
public:
    EchoServer(EventLoop* loop, Address addr, SocketOptions opts)
        : TCPServer(loop, addr, opts)
        , _cq(4)
    {
        _cq.resize(4);
        loop->run_every(1s, std::bind(&EchoServer::OnTimer, this));
    }

    virtual void OnConnection(TCPConnection::ptr conn) override;
    virtual void OnMessage(TCPConnection::ptr conn, Buffer& buffer) override;

    void OnTimer() {
        _cq.push_back(Bucket());
    }
private:
    struct Entry {
        Entry(weak_ptr<TCPConnection> wptr)
            : _wptr(wptr)
        {}
        
        ~Entry() {
            auto ptr = _wptr.lock();
            if (ptr) {
                ptr->force_close();
            }
        }
        weak_ptr<TCPConnection> _wptr;
    };

    using EntryPtr = shared_ptr<Entry>;
    using EntryWeakPtr = weak_ptr<Entry>;
    using Bucket = unordered_set<EntryPtr>;
    boost::circular_buffer<Bucket> _cq;
};

void EchoServer::OnConnection(TCPConnection::ptr conn) {
    // if (conn->connected()) {
    //     LOG_INFO << "New connection from ["
    //         << conn->get_peer_address().ip()
    //         << ":" << conn->get_peer_address().port()
    //         << "] id = " << g_total_conns << "\n";
    // } else {
    //     LOG_INFO << "Connection from ["
    //         << conn->get_peer_address().ip()
    //         << ":" << conn->get_peer_address().port()
    //         << "] is closed!\n";
    // }
    auto entry = make_shared<Entry>(conn);
    _cq.back().insert(entry);
    EntryWeakPtr weak_entry(entry);
    conn->set_context(weak_entry);
}

HttpResponse res(HttpStatus::OK);
string g_str = res.to_string();

void EchoServer::OnMessage(TCPConnection::ptr conn, Buffer& buffer) {
    g_total_conns++;
    std::string msg1 = buffer.peek_all();
    conn->send(g_str);
    conn->force_close();
    // cout << g_total_conns << endl;
    EntryWeakPtr entry_weak(std::any_cast<EntryWeakPtr>(conn->get_context()));
    EntryPtr entry_ptr(entry_weak.lock());
    if (entry_ptr)
        _cq.back().insert(entry_ptr);
}

// AsyncLogInit mylog("/tmp/log");
int main() {
    cout << "response:\n" << g_str;
    Logger::SetBufferLevel(Logger::kLineBuffer);
    LOG_INFO << "pid = " << (int)CurrentThread::gettid() << "\n";
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
