#include "socket.h"
#include "flog.h"
#include "config.h"
#include <sys/types.h>
#include <socket.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#include <filesystem>

namespace fs = std::filesystem;

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

void SocketOptions::set_options(int fd) {
    // default is blocking
    // LOG_INFO << "fd = " << fd << "\n";
    if (!blocking) {
        int args = syscall_ret("fcntl", ::fcntl(fd, F_GETFL));
        args |= O_NONBLOCK;
        syscall("fcntl", ::fcntl(fd, F_SETFL, args));
    }

    if (reuseaddr) {
        int val = 1;
        socklen_t optlen = sizeof val;
        syscall("setsockopt", 
                ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, optlen));
    }

    if (reuseport) {
        int val = 1;
        socklen_t optlen = sizeof val;
        syscall("setsockopt", 
                ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, optlen));
    }

    if (keepalive) {
        int val = 1;
        socklen_t optlen = sizeof val;
        syscall("setsockopt", 
                ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, optlen));
        
        syscall("setsockopt", 
                ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &tcp_keepcnt, sizeof(int)));
        syscall("setsockopt", 
                ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepidle, sizeof(int)));
        syscall("setsockopt", 
                ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &tcp_keepinterval, sizeof(int)));
    }

    if (tcp_nodelay) {
        int val = 1;
        socklen_t optlen = sizeof val;
        syscall("setsockopt", 
                ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, optlen));
    }

    // use_ssl
    if (use_ssl) {
        static int ret = Socket::ssl_init();
    }
}

// -----------------------------------------------------------------------------------------Socket
static ConfigVar<size_t>::ptr recv_size_limited = Config::lookup("srpc.net.socket.Socket.size_limited", 
        static_cast<size_t>(65536), "default maximum receive size is 64KB");
Socket::Socket(int fd, const SocketOptions& o)
    : FDescriptor(fd)
    , opts(o)
{
    opts.set_options(fd);
}

Socket::Socket(int domain, int type, const SocketOptions& o)
    : Socket(::socket(domain, type, 0), o)
{}

Socket::~Socket() {
    if (_ssl) {
        SSL_shutdown(_ssl);
        SSL_free(_ssl);
        if (_ssl_ctx)
            SSL_CTX_free(_ssl_ctx);
    }
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

    // ssl
    if (opts.use_ssl) {
        _ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (_ssl_ctx == NULL) {
            LOG_ERROR << "SSL_CTX_new error!\n";
            return;
        }

        // 对证书进行验证
        // SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_PEER, NULL);
        // 设置信任根证书
        // if (SSL_CTX_load_verify_locations(_ssl_ctx, "/home/cwp/ca/certs/ca.cert.pem", NULL) <= 0) {
        //     ERR_print_errors_fp(stdout);
        //     return;
        // }
        _ssl = SSL_new(_ssl_ctx);
        if (_ssl == NULL) {
            LOG_ERROR << "SSL_new error!\n";
            return;
        }

        SSL_set_fd(_ssl, fd());

        SSL_connect(_ssl);
    }
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
    // ssl
    if (opts.use_ssl) {
        if (SSL_write(_ssl, msg, ::strlen(msg)) <= 0) {
            LOG_WARN << "SSL_write is not success!\n";
            return;
        }
    } else
        syscall("Socket::send", ::send(fd(), msg, ::strlen(msg), 0));
}
void SSL_error_info(int ret) {
    switch(ret) {
        case SSL_ERROR_ZERO_RETURN:
            LOG_INFO << "No more data can be read!\n";
            break;
        case SSL_ERROR_WANT_READ:
            LOG_INFO << "The operation did not complete and can be retried later.\n";
            break;
        case SSL_ERROR_WANT_X509_LOOKUP:
            LOG_INFO << "SSL_ERROR_WANT_X509_LOOKUP\n";
            break;
        case SSL_ERROR_WANT_ASYNC:
            LOG_INFO << "SSL_ERROR_WANT_ASYNC\n";
            break;
        case SSL_ERROR_WANT_ASYNC_JOB:
            LOG_INFO << "SSL_ERROR_WANT_ASYNC_JOB\n";
            break;
        case SSL_ERROR_WANT_CLIENT_HELLO_CB:
            LOG_INFO << "SSL_ERROR_WANT_CLIENT_HELLO_CB\n";
            break;
        case SSL_ERROR_SYSCALL:
            LOG_INFO << "SSL_ERROR_SYSCALL\n"
                << "errno = " << errno
                << ", strerror: " << ::strerror(errno);
            break;
        case SSL_ERROR_SSL:
            LOG_INFO << "SSL_ERROR_SSL\n";
            break;
        case SSL_ERROR_NONE:
            LOG_INFO << "The TLS/SSL I/O operation completed.\n";
            break;
        default:
            LOG_WARN << "SSL_read is not success!\n";
    }
}
std::string Socket::recv(size_t limited) {
    size_t recv_size = std::min(recv_size_limited->getValue(), limited);
    char buf[recv_size + 1]{};
    if (opts.use_ssl) {
        int ret = SSL_read(_ssl, buf, sizeof buf);
        if (ret <= 0) {
            SSL_error_info(SSL_get_error(_ssl, ret));
            return {};
        }
    } else
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

int Socket::ssl_init() {
#if OPENSSL_VERSION_NUMBER >= 0x10100003L

    if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL) == 0)
    {
        LOG_ERROR << "OPENSSL_init_ssl() failed!\n";
        return -1;
    }

    /*
     * OPENSSL_init_ssl() may leave errors in the error queue
     * while returning success
     */

    ERR_clear_error();

#else

    OPENSSL_config(NULL);

    SSL_library_init();         // 初始化SSL算法库函数( 加载要用到的算法 )，调用SSL函数之前必须调用此函数
    SSL_load_error_strings();   // 错误信息的初始化

    OpenSSL_add_all_algorithms();

#endif
    return 0;
}
// -----------------------------------------------------------------------------------------UDPSocket
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
TCPSocket::TCPSocket(int fd, const SocketOptions& opts)
    : Socket(fd, opts)
{}

TCPSocket::TCPSocket(const SocketOptions& opts)
    : Socket(AF_INET, SOCK_STREAM, opts)
{}

TCPSocket::TCPSocket(int fd, const SocketOptions& opts, SSL_CTX* ctx, SSL* ssl) 
    : Socket(fd, opts)
{
    _ssl_ctx = ctx;
    _ssl = ssl;
}

static ConfigVar<std::string>::ptr tcpsocket_certificate_path = 
        Config::lookup("srpc.net.socket.tcpsocket_certificate_path", 
                static_cast<std::string>("/tmp/xxx.PEM"), "path of certificate.");

static ConfigVar<std::string>::ptr tcpsocket_private_key_path = 
        Config::lookup("srpc.net.socket.tcpsocket_private_key_path", 
                static_cast<std::string>("/tmp/xxx_pk.PEM"), "path of private key.");

void TCPSocket::listen() {
    if (opts.use_ssl) {
        _ssl_ctx = SSL_CTX_new(TLS_server_method());
        if (_ssl_ctx == NULL) {
            LOG_ERROR << "SSL_CTX_new error! Communication is not safe!\n";
            return;
        }
        // load certificate chains
        if (SSL_CTX_use_certificate_file(_ssl_ctx, tcpsocket_certificate_path->getValue().c_str(), SSL_FILETYPE_PEM) != 1) 
        {
            LOG_ERROR << "SSL_CTX_use_certificate_chain_file error! Communication is not safe!\n";
            return;
        }
        // load private key
        if (1 != SSL_CTX_use_PrivateKey_file(_ssl_ctx, tcpsocket_private_key_path->getValue().c_str(), SSL_FILETYPE_PEM)) {
            LOG_ERROR << "SSL_CTX_use_PrivateKey_file error! Communication is not safe!\n";
            return;
        }
        // check private key
        if (1 != SSL_CTX_check_private_key(_ssl_ctx)) {
            LOG_ERROR << "SSL_CTX_check_private_key error! Communication is not safe!\n";
            return;
        }
    }

    syscall("TCPSocket::listen", ::listen(fd(), 10));
}

TCPSocket TCPSocket::accept() {
    int ret = syscall_ret("TCPSocket::accept", ::accept(fd(), NULL, NULL));
    if (ret < 0) return {};
    
    if (opts.use_ssl && _ssl_ctx) {
        _ssl = SSL_new(_ssl_ctx);
        if (_ssl == NULL) {
            LOG_ERROR << "SSL_new error!\n";
            return {};
        }
        SSL_set_fd(_ssl, ret);
        int rv = SSL_accept(_ssl);
        if (rv == -1) {
            LOG_ERROR << "ssl accept\n";
            SSL_error_info(SSL_get_error(_ssl, rv));
            ::exit(1);
        }
        return {ret, get_socket_options(), NULL, _ssl};
    }
    return {ret, get_socket_options()};
}

// -----------------------------------------------------------------------------------------UnixSocket
UnixSocket::UnixSocket(int fd, const SocketOptions& opts)
    : Socket(fd, opts)
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
    return {ret, get_socket_options()};
}
