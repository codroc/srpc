#include "socket.h"
#include "flog.h"
#include "config.h"
#include <sys/types.h>
#include <socket.h>
#include <string.h>
#include <errno.h>


// -----------------------------------------------------------------------------------------Socket
static ConfigVar<size_t>::ptr recv_size_limited = Config::lookup("srpc.net.socket.Socket.size_limited", 
        static_cast<size_t>(65536), "default maximum receive size is 64KB");
Socket::Socket(int domain, int type) 
    : FDescriptor(::socket(domain, type, 0))
{}

Socket::Socket(int fd)
    : FDescriptor(fd)
{}

Socket::~Socket() {
}
void syscall(const std::string& description, int ret) {
    if (-1 == ret) {
        LOG_ERROR << description 
            << "\t"
            << ::strerror(errno) << "\n";
    }
}
// brief: the syscall which has meaningful ret value
int syscall_ret(const std::string& description, int ret) {
    if (-1 == ret) {
        LOG_ERROR << description 
            << "\t"
            << ::strerror(errno) << "\n";
    }
    return ret;
}
void Socket::bind(const Address& addr) {
    syscall("Socket::bind", ::bind(fd(), addr, addr.size()));
}
void Socket::bind(const std::string& ip, uint16_t port) {
    Address addr(ip, port);
    bind(addr);
}
void Socket::bind(const std::string& ip, const std::string& port) {
    Address addr(ip, port);
    bind(addr);
}

void Socket::connect(const Address& addr) {
    syscall("Socket::connect", ::connect(fd(), addr, addr.size()));
}

void Socket::connect(const std::string& ip, uint16_t port) {
    Address addr(ip, port);
    connect(addr);
}

void Socket::connect(const std::string& ip, const std::string& port) {
    Address addr(ip, port);
    connect(addr);
}

void Socket::sendto(const char* msg, const Address& addr) {
    syscall("Socket::sendto", ::sendto(fd(), msg, ::strlen(msg), 0, addr, addr.size()));
}

void Socket::sendto(const std::string& msg, const Address& addr) {
    sendto(msg.c_str(), addr);
}

void Socket::sendto(const std::string& msg, const std::string& ip, uint16_t port) {
    sendto(msg, {ip, port});
}

void Socket::sendto(const std::string& msg, const std::string& ip, const std::string& port) {
    sendto(msg, {ip, port});
}

void Socket::send(const std::string& msg) {
    // can also see in file /proc/sys/net/core/wmem_default or 
    // /proc/sys/net/core/wmem_max
    // size_t max_snd_buf{};
    // socklen_t optlen{};
    // syscall("getsockopt", ::getsockopt(fd(), SOL_SOCKET, SO_SNDBUF, &max_snd_buf, &optlen));
    // if (msg.size() > max_snd_buf) {
    //     LOG_ERROR << "socket maximum send buffer size is " 
    //         << max_snd_buf
    //         << " but, you are going to send "
    //         << msg.size() << " bytes!\n";
    //     return;
    // }
    send(msg.c_str());
}
void Socket::send(const char* msg) {
    size_t max_snd_buf{};
    socklen_t optlen = sizeof max_snd_buf;
    syscall("getsockopt", ::getsockopt(fd(), SOL_SOCKET, SO_SNDBUF, &max_snd_buf, &optlen));
    if (::strlen(msg) > max_snd_buf) {
        LOG_ERROR << "socket maximum send buffer size is " 
            << max_snd_buf
            << " but, you are going to send "
            << ::strlen(msg) << " bytes!\n";
        return;
    }
    syscall("Socket::send", ::send(fd(), msg, ::strlen(msg), 0));
}

std::string Socket::recv(size_t limited) {
    size_t recv_size = std::min(recv_size_limited->getValue(), limited);
    char buf[recv_size + 1]{};
    syscall("Socket::recv", ::recv(fd(), buf, sizeof buf, 0));
    return buf;
}

Address Socket::get_local_address() const {
    struct sockaddr addr;
    socklen_t len = sizeof addr;
    syscall("Socket::get_local_address", ::getsockname(fd(), &addr, &len));
    return {&addr, len};
}
Address Socket::get_peer_address() const {
    struct sockaddr addr;
    socklen_t len = sizeof addr;
    syscall("Socket::get_peer_address", ::getpeername(fd(), &addr, &len));
    return {&addr, len};
}

// -----------------------------------------------------------------------------------------UDPSocket
UDPSocket::UDPSocket() 
    : Socket(AF_INET, SOCK_DGRAM)
{}

std::pair<std::string, Address> UDPSocket::recvfrom(size_t limited) {

    size_t recv_size = std::min(recv_size_limited->getValue(), limited);
    char buf[recv_size + 1]{};

    struct sockaddr addr{};
    socklen_t len = sizeof addr;
    syscall("UDPSocket::recvfrom", ::recvfrom(fd(), buf, sizeof buf, 0, &addr, &len));
    return {buf, {&addr, len}};
}

// -----------------------------------------------------------------------------------------TCPSocket
TCPSocket::TCPSocket() 
    : Socket(AF_INET, SOCK_STREAM)
{}

TCPSocket::TCPSocket(int fd) 
    : Socket(fd)
{}

void TCPSocket::listen() {
    syscall("TCPSocket::listen", ::listen(fd(), 10));
}

TCPSocket TCPSocket::accept() {
    int ret = syscall_ret("TCPSocket::accept", ::accept(fd(), NULL, NULL));
    return {ret};
}

// -----------------------------------------------------------------------------------------UnixSocket
UnixSocket::UnixSocket()
    : Socket(AF_UNIX, SOCK_SEQPACKET)
{}

UnixSocket::UnixSocket(int fd)
    : Socket(fd)
{}

void UnixSocket::bind(const std::string& pathname) {
    Address addr(pathname);
    Socket::bind(addr);
}

void UnixSocket::listen() {
    syscall("UnixSocket::listen", ::listen(fd(), 10));
}

UnixSocket UnixSocket::accept() {
    int ret = syscall_ret("UnixSocket::accept", ::accept(fd(), NULL, NULL));
    return {ret};
}
