#include "flog.h"

#include "address.h"
#include "config.h"
#include <string.h>
#include <string>


// static ConfigVar<std::string>::ptr unix_domain_pathname = Config::lookup(
//         "srpc.net.address.Address.unix_domain_pathname",
//         static_cast<std::string>("/tmp/unix.socket", "unix domain socket pathname"));

Address::Address(const std::string& host, const std::string& service, 
            const addrinfo& hits)
{
    struct addrinfo* ai = nullptr;
    auto deleter = [](addrinfo *const x) { ::freeaddrinfo(x); };
    int ret = ::getaddrinfo(host.c_str(), service.c_str(), 
                                &hits, &ai);
    std::unique_ptr<addrinfo, decltype(deleter)> up(ai, deleter);
    if (ai == nullptr)
        LOG_ERROR << "getaddrinfo return zero addrinfo\n";
    *this = Address(ai->ai_addr, ai->ai_addrlen);
}

Address::Address(const std::string& host, const std::string& service) 
    : Address(host, service, make_hits(AI_ALL, AF_INET))
{}

Address::Address(const std::string& ip, const uint16_t port)
    : Address(ip, std::to_string(port), make_hits(/*AI_NUMERICHOST |*/ AI_NUMERICSERV, AF_INET))
{}

Address::Address(const sockaddr* addr, const socklen_t size) {
    ::memcpy(_storage, addr, size);
    _size = size;
}

Address::Address(const std::string& pathname)
    : Address(pathname.c_str())
{}

Address::Address(const char* pathname) {
    ::unlink(pathname);
    struct sockaddr_un un{};
    socklen_t len = sizeof un;
    un.sun_family = AF_UNIX;
    ::memmove(un.sun_path, pathname, ::strlen(pathname));
    *this = Address((sockaddr*)&un, len);
}

std::pair<std::string, uint16_t> Address::ip_port() const {
    const struct sockaddr_in* addr = reinterpret_cast<const struct sockaddr_in*>(&_storage);
    return {::inet_ntoa(addr->sin_addr), ::ntohs(addr->sin_port)};
}

const struct addrinfo Address::make_hits(int ai_flags, int ai_family) {
    struct addrinfo hits{};
    hits.ai_flags = ai_flags;
    hits.ai_family = ai_family;
    return hits;
}
