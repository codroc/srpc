#include "socket.h"
#include "flog.h"
#include "config.h"
#include <sys/types.h>
#include <socket.h>
#include <string.h>
#include <errno.h>

#include <filesystem>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------------------SocketOptions
static ConfigVar<std::string>::ptr socket_options_config_file = Config::lookup("srpc.net.socket.SocketOptions.config_file", 

        static_cast<std::string>("options.yaml"), "config file of socket options");
static ConfigVar<bool>::ptr socket_options_blocking = Config::lookup("srpc.net.socket.SocketOptions.blocking", 
        static_cast<bool>(true), "blocking socket");
static ConfigVar<bool>::ptr socket_options_reuseaddr = Config::lookup("srpc.net.socket.SocketOptions.reuseaddr", 
        static_cast<bool>(false), "not reuse addr");
static ConfigVar<bool>::ptr socket_options_reuseport = Config::lookup("srpc.net.socket.SocketOptions.reuseport", 
        static_cast<bool>(false), "not reuse port");

static ConfigVar<bool>::ptr socket_options_keepalive = Config::lookup("srpc.net.socket.SocketOptions.keepalive", 
        static_cast<bool>(false), "not keepalive");
static ConfigVar<int>::ptr socket_options_tcp_keepcnt = Config::lookup("srpc.net.socket.SocketOptions.tcp_keepcnt", 
        static_cast<int>(5), "default retx times");
static ConfigVar<int>::ptr socket_options_tcp_keepidle = Config::lookup("srpc.net.socket.SocketOptions.tcp_keepidle", 
        static_cast<int>(5), "default idle time");
static ConfigVar<int>::ptr socket_options_tcp_keepinterval = Config::lookup("srpc.net.socket.SocketOptions.tcp_keepinterval", 
        static_cast<int>(5), "default interval time");

static ConfigVar<bool>::ptr socket_options_tcp_nodelay = Config::lookup("srpc.net.socket.SocketOptions.tcp_nodelay", 
        static_cast<bool>(false), "false means: send package as big as possible");

static int CalledOnce() {
    LOG_INFO << "please create options.yaml and do some config!\n";
    return 0;
}

static void LoadFromCfg() {
    std::string config_file = socket_options_config_file->getValue();
    fs::path path{config_file};
    fs::directory_entry entry{path};
    if (!entry.exists()) {
        static int a = CalledOnce();
    } else {
        Config::loadFromYaml(config_file);
    }
}

SocketOptions::SocketOptions() {
    LoadFromCfg();
    blocking = socket_options_blocking->getValue();

    reuseaddr = socket_options_reuseaddr->getValue();
    reuseport = socket_options_reuseport->getValue();

    keepalive = socket_options_keepalive->getValue();
    tcp_keepcnt = socket_options_tcp_keepcnt->getValue();
    tcp_keepidle = socket_options_tcp_keepidle->getValue();
    tcp_keepinterval = socket_options_tcp_keepinterval->getValue();

    tcp_nodelay = socket_options_tcp_nodelay->getValue();
}


// -----------------------------------------------------------------------------------------Socket
static ConfigVar<size_t>::ptr recv_size_limited = Config::lookup("srpc.net.socket.Socket.size_limited", 
        static_cast<size_t>(65536), "default maximum receive size is 64KB");
Socket::Socket(int domain, int type) 
    : FDescriptor(::socket(domain, type, 0))
    , opts()
{}

Socket::Socket(int fd)
    : FDescriptor(fd)
    , opts()
{}

Socket::Socket(int domain, int type, const SocketOptions& o)
    : FDescriptor(::socket(domain, type, 0))
    , opts(o)
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

UDPSocket::UDPSocket(const SocketOptions& opts)
    : Socket(AF_INET, SOCK_DGRAM, opts)
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

TCPSocket::TCPSocket(const SocketOptions& opts)
    : Socket(AF_INET, SOCK_STREAM, opts)
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
 
UnixSocket::UnixSocket(const SocketOptions& opts)
    : Socket(AF_UNIX, SOCK_SEQPACKET, opts)
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
