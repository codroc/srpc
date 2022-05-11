#ifndef SRPC_NET_SOCKET_H
#define SRPC_NET_SOCKET_H

#include "file_descriptor.h"
#include "address.h"

// ssl
class SslSocketImpl;
#include <openssl/ssl.h>

#include <memory>
#include <string>

// brief: socket options
// The socket options listed below can be set by using setsockopt(2) and read with getsockopt(2) 
// with the socket level  set  to  SOL_SOCKET  for  all sockets.
struct SocketOptions {
    SocketOptions();

    // fcntl
    bool blocking{};

    // setsockopt and getsockopt
    // option level: SOL_SOCKET
    bool reuseaddr{};
    bool reuseport{};

    // brief: Enable sending of keep-alive messages on connection-oriented sockets. (ref: man7::socket)
    bool keepalive{};
    
    // option level: IPPROTO_TCP
    
    // brief: The maximum number of keepalive probes TCP should send before dropping the connection.
    // This option should not be used in code intended to be portable.
    int tcp_keepcnt{};

    // brief: The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes, if the socket option SO_KEEPALIVE has been set on this socket.
    // This  option should not be used in code intended to be portable.
    int tcp_keepidle{};

    // brief: The time (in seconds) between individual keepalive probes.  This option should not be used in code intended to be portable.
    int tcp_keepinterval{};

    // brief: If set, disable the Nagle algorithm. (ref: man7::tcp)
    bool tcp_nodelay{};

    // suport ssl
    // just suport tcp now
    bool use_ssl{};

    // brief: set socket options
    void set_options(int fd);
};

class Socket : public FDescriptor {
public:
    SocketOptions opts;
    SocketOptions& get_socket_options() { return opts; }
    const SocketOptions& get_socket_options() const { return opts; }

    // int socket(int domain, int type, int protocol); [socket(2)](ref man::socket)
    // Socket(int domain, int type);
    Socket(int domain, int type, const SocketOptions& o = SocketOptions());
    Socket(int fd, const SocketOptions& o);
    virtual ~Socket();

    // brief: bind to an address, can used by UDPSocket and TCPSocket.
    // param addr: Address class is an abstruct of (ip, port)/(ip, service)/(hostname, port)/(hostname, service)/("/tmp/xxx")
    void bind(const Address& addr);
    void bind(const std::string& ip, uint16_t port);
    void bind(const std::string& ip, const std::string& port);

    // brief: connect to an address, can used by UDPSocket and TCPSocket. [connect(2)](ref man::connect)
    // UDP 也是可以调用 connect 的，本质是，OS 将记住 peer 的 ip:port，下次可以直接调用 send 或 recv
    void connect(const Address& addr);
    void connect(const std::string& ip, uint16_t port);
    void connect(const std::string& ip, const std::string& port);
    
    // write bytes to socket

    // brief: send msg to an address, can be override by UDPSocket.
    // param msg: msg which is going to be sent
    // param addr: Address class is an abstruct of (ip, port)/(ip, service)/(hostname, port)/(hostname, service)
    void sendto(const std::string& msg, const Address& addr);
    void sendto(const std::string& msg, const std::string& ip, uint16_t port);
    void sendto(const std::string& msg, const std::string& ip, const std::string& port);
    // overload: accept c-type string
    void sendto(const char* msg, const Address& addr);

    // brief: send msg to peer, can be override by TCPSocket and UDPSocket.
    // Because tcp has connection, we don't have to specifiy an address. But udp must connect before call send
    // param msg: msg which is going to be sent
    ssize_t send(const std::string& msg);
    ssize_t send(std::string_view msg);
    // overload: accept c-type string
    ssize_t send(const char* msg);

    std::string recv(size_t limited = std::numeric_limits<size_t>::max());
    ssize_t recv(std::string& str, size_t limited = std::numeric_limits<size_t>::max());

    // brief: get local address
    Address get_local_address() const;
    // brief: get peer address
    Address get_peer_address() const;
protected:
    std::shared_ptr<SslSocketImpl> _sss_impl;
};

// Note: The appropriate size of UDP datagram is 576 bytes.
// see detial on https://blog.csdn.net/luojian5900339/article/details/78472137
class UDPSocket : public Socket {
public:
    using ptr = std::shared_ptr<UDPSocket>;
    UDPSocket(const SocketOptions& opts = SocketOptions());

    // brief: receive msg from socket
    // param limited: maximum bytes we can accept
    std::pair<std::string, Address> recvfrom(size_t limited = std::numeric_limits<size_t>::max());
};

class TCPSocket : public Socket {
private:
    // brief: used by accept
    // TCPSocket(int fd);
    TCPSocket(int fd, const SocketOptions& opts);

    //brief: for accept a ssl sock
    TCPSocket(int fd, const SocketOptions& opts, SSL_CTX* ctx, SSL* ssl);
    // TCPSocket(int fd, const SocketOptions& opts, const SslSocketImpl& sss_impl);
public:
    using ptr = std::shared_ptr<TCPSocket>;
    // TCPSocket();
    TCPSocket(const SocketOptions& opts = SocketOptions());

    // brief: listen on port [listen(2)](ref: man::listen)
    void listen();
    // brief: accept a completely established socket from listen socket's queue
    TCPSocket accept();
};

class UnixSocket : public Socket {
private:
    // brief: used by accept
    UnixSocket(int fd, const SocketOptions& opts);
public:
    using ptr = std::shared_ptr<UnixSocket>;
    // brief: use SOCK_UNIX domain
    // and SOCK_SEQPACKET socket type. This is a connection-orianted and message bounded 
    // socket type. So the program just like tcp
    // server: socket, bind, listen, accept, read\write, close
    // client: socket, connect, read\write, close
    // [Unix domain socket; local stream IPC](ref: man7::unix)
    UnixSocket(const SocketOptions& opts = SocketOptions());
    void bind(const std::string& pathname);
    void listen();
    UnixSocket accept();
};
#endif
