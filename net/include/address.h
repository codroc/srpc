#ifndef SRPC_NET_ADDRESS_H
#define SRPC_NET_ADDRESS_H

#include <memory>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Address {
public:
    using ptr = std::shared_ptr<Address>;
    struct Raw {
        sockaddr_storage storage{};
        operator sockaddr* () { return reinterpret_cast<sockaddr*> (&storage); }
        operator const sockaddr* () { return reinterpret_cast<const sockaddr*> (&storage); }
    };
    // brief: This constr provide dns service by getaddrinfo
    // param host: host name such as "google.com"
    // param service: service name such as "http", "https" ...
    Address(const std::string& host, const std::string& service);

    // brief: Construct by 192.168.128.138:80
    // param ip: dotted-quad string
    // param port: numberic port
    Address(const std::string& ip, const uint16_t port);

    // brief: Construct from [sockaddr*]
    Address(const sockaddr* addr, const socklen_t size);

    // brief: Get Address's ip and port
    // return: first  - dotted-quad string such as "127.0.0.1"
    //         second - numberic port
    std::pair<std::string, uint16_t> ip_port() const;

    // brief: Get dotted-quad IP address such as "127.0.0.1"
    std::string ip() const { return ip_port().first; }

    // brief: Get numberic port such as 80
    uint16_t port() const { return ip_port().second; }

    // brief: Get Human-readable string e.g., "127.0.0.1:80"
    std::string to_string() const { return ip() + ":" + std::to_string(port()); }

    socklen_t size() const { return _size; }

private:
    Address(const std::string& host, const std::string& service, 
            const struct addrinfo& hits);
    const struct addrinfo make_hits(int ai_flag, int ai_family);
private:
    Raw _storage{};
    socklen_t _size;
};

#endif
