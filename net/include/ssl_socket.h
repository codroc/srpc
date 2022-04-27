#ifndef SRPC_NET_SSL_SOCKET_H
#define SRPC_NET_SSL_SOCKET_H

#include "socket.h"
#include <memory>

// ssl
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/evp.h>


class SslSocketImpl {
public:
    using ptr = std::shared_ptr<SslSocketImpl>;
    SslSocketImpl() = default;
    SslSocketImpl& operator=(const SslSocketImpl&) = default;
    ~SslSocketImpl();
    SslSocketImpl(SSL_CTX* ctx, SSL* ssl)
        : _ssl_ctx(ctx)
        , _ssl(ssl)
    {}

    // brief: init ssl env
    static int ssl_init();

    // brief: init SslSocketImpl object by object
    // param rhs: the object
    // void init(const SslSocketImpl& rhs);
    
    // brief: init SslSocketImpl object by concret ssl implementation
    // param ctx: ssl context
    // param ssl: 
    void init(SSL_CTX* ctx, SSL* ssl);

    // brief: call SSL_connect
    // param fd: file descriptor
    void connect(int fd);

    // brief: call SSL_listen
    void listen();

    // brief: call accept
    // param fd: file descriptor
    SSL* accept(int fd);

    // brief: send a msg by SSL_write
    size_t send(const char* msg);
    size_t send(const std::string& msg);

    // brief: recv a msg safely
    // param buf: data will be fill in buf
    // param len: max size of buf
    int recv(char* buf, size_t len);
private:
    SSL_CTX* _ssl_ctx{};
    SSL*     _ssl{};
};

#endif

