// 这是一个 NIO + IO multiplexing 的 discard 多线程程序
#include "net.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <map>
#include <iostream>
#include <string>
#include <memory>

using TCPSocketPtr = std::shared_ptr<TCPSocket>;
using ChannelPtr = std::shared_ptr<Channel>;
std::map<int, TCPSocketPtr> g_sockets;
std::map<int, ChannelPtr> g_channels;

// fix me: 把 TCPSocket 和 Channel 封装到 TCPConnection 类中去
void handle_read(EventLoop* loop, TCPSocketPtr sock, ChannelPtr channel) {
    std::string str = sock->recv();
    int n = str.size();
    if (n < 0) { // fix me: string::size 是不可能返回 n < 0 这种情况的。这里怎么办？
        std::cout << "read error! str error: "
                  << ::strerror(errno) << "\n";
        exit(-1);
    } else if (n == 0) {
        std::cout << "connection from [" << sock->get_peer_address().ip()
             << ":" << sock->get_peer_address().port()
             << "] closed!\n";
        channel->set_events(Channel::kNoneEvent);
        channel->updata();
    } else {
        std::cout << "connection [" << sock->get_peer_address().ip()
             << ":" << sock->get_peer_address().port()
             << "]" << " sock fd: " << sock->fd() << " received "
            << n << " bytes!\n";
    }
}

void handle_accept(EventLoop* loop, TCPSocket* serv) {
    TCPSocketPtr client = std::make_shared<TCPSocket>(serv->accept());
    int fd = client->fd();
    if (fd >= 0) {
        std::cout << "new connection from [" << client->get_peer_address().ip()
             << ":" << client->get_peer_address().port()
             << "]\n";
        ChannelPtr channel = std::make_shared<Channel>(loop, fd);   
        // fix me: 这里因为 client 和 channel 的生命周期要超过当前作用域
        // 如何优雅的延长这两者的生命周期？
        g_sockets.insert({fd, client});
        g_channels.insert({fd, channel});

        // fix me: 如果用 map::operator[] 那么一定要求 value 具有默认构造函数
        // 很明显，我不想 Channel 存在 Channel::Channel() 那么只能用 find 的方法吗？
        g_channels.find(fd)->second->set_events(Channel::kReadEvent);
        g_channels.find(fd)->second->set_read_callback(std::bind(handle_read, loop, g_sockets.find(fd)->second, g_channels.find(fd)->second));
        g_channels.find(fd)->second->set_error_callback([]() {
                std::cout << "error!\n";
                exit(-1);
                });
        g_channels.find(fd)->second->updata();
    } else {
        std::cout << "accept error!\n";
        exit(-1);
    }
}

int main(int argc, char** argv) {
    SocketOptions opts;
    opts.blocking = false; // 非阻塞
    opts.reuseaddr = true;

    TCPSocket serv(opts);
    serv.bind({"127.0.0.1", 8080});

    EventLoop loop;
    Channel channel(&loop, serv.fd());
    channel.set_events(Channel::kReadEvent);
    channel.set_read_callback(std::bind(handle_accept, &loop, &serv));
    channel.set_error_callback([](){
            std::cout << "error!\n";
            exit(-1);
            });
    channel.updata();
    // g_channels.insert({serv.fd(), channel});

    serv.listen();
    loop.loop();

    return 0;
}
