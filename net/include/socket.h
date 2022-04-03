#ifndef SRPC_NET_SOCKET_H
#define SRPC_NET_SOCKET_H

#include "file_descriptor.h"

#include <memory>

class Socket : public FDescriptor {
public:
    // int socket(int domain, int type, int protocol); [socket(2)](ref man::socket)
    Socket(int domain, int type);

    // brief: bind to an address, can be override by UDPSocket and TCPSocket.
    // param addr: Address class is an abstruct of (ip, port)/(ip, service)/(hostname, port)/(hostname, service)/("/tmp/xxx")
    virtual void bind(const Address& addr);

    // brief: connect to an address, can be override by UDPSocket and TCPSocket. [connect(2)](ref man::connect)
    // UDP 也是可以调用 connect 的，本质是，OS 将记住 peer 的 ip:port，下次可以直接调用 send 或 recv
    virtual void connect(const Address& addr);
    
    // write bytes to socket

    // brief: send msg to an address, can be override by UDPSocket.
    // param msg: msg which is going to be sent
    // param addr: Address class is an abstruct of (ip, port)/(ip, service)/(hostname, port)/(hostname, service)
    virtual void sendto(const std::string& msg, const Address& addr);
    // overload: accept c-type string
    virtual void sendto(const char* msg, const Address& addr);

    // brief: send msg to peer, can be override by TCPSocket and UDPSocket.
    // Because tcp has connection, we don't have to specifiy an address. But udp must connect before call send
    // param msg: msg which is going to be sent
    virtual void send(const std::string& msg);
    // overload: accept c-type string
    virtual void send(const char* msg);

    // read bytes from socket
    
    // brief: receive msg from socket, can be override by UDPSocket and TCPSocket.
    // param limited: maximum bytes we can accept
    virtual std::string recvfrom(size_t limited = std::numeric_limits<size_t>::max());
};

#endif
