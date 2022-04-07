#ifndef SRPC_NET_SOCKET_H
#define SRPC_NET_SOCKET_H

#include "file_descriptor.h"
#include "address.h"

#include <memory>
#include <string>

class Socket : public FDescriptor {
public:
    // int socket(int domain, int type, int protocol); [socket(2)](ref man::socket)
    Socket(int domain, int type);
    Socket(int fd);
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
    void send(const std::string& msg);
    // overload: accept c-type string
    void send(const char* msg);

    std::string recv(size_t limited = std::numeric_limits<size_t>::max());

    // brief: get local address
    Address get_local_address() const;
    // brief: get peer address
    Address get_peer_address() const;
};

// Note: The appropriate size of UDP datagram is 576 bytes.
// see detial on https://blog.csdn.net/luojian5900339/article/details/78472137
class UDPSocket : public Socket {
public:
    UDPSocket();

    // brief: receive msg from socket
    // param limited: maximum bytes we can accept
    std::pair<std::string, Address> recvfrom(size_t limited = std::numeric_limits<size_t>::max());
};

class TCPSocket : public Socket {
private:
    // brief: used by accept
    TCPSocket(int fd);
public:
    TCPSocket();

    // brief: listen on port [listen(2)](ref: man::listen)
    void listen();
    // brief: accept a completely established socket from listen socket's queue
    TCPSocket accept();
};

class UnixSocket : public Socket {
private:
    // brief: used by accept
    UnixSocket(int fd);
public:
    // brief: use SOCK_UNIX domain
    // and SOCK_SEQPACKET socket type. This is a connection-orianted and message bounded 
    // socket type. So the program just like tcp
    // server: socket, bind, listen, accept, read\write, close
    // client: socket, connect, read\write, close
    // [Unix domain socket; local stream IPC](ref: man7::unix)
    UnixSocket();
    void bind(const std::string& pathname);
    void listen();
    UnixSocket accept();
};
#endif
