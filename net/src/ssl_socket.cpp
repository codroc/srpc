#include "ssl_socket.h"
#include "config.h"
#include "flog.h"

#include <sys/types.h>
#include <socket.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <filesystem>


static ConfigVar<std::string>::ptr tcpsocket_certificate_path =
Config::lookup("srpc.net.socket.tcpsocket_certificate_path",
        static_cast<std::string>("/tmp/xxx.PEM"), "path of certificate.");

static ConfigVar<std::string>::ptr tcpsocket_private_key_path =
Config::lookup("srpc.net.socket.tcpsocket_private_key_path",
        static_cast<std::string>("/tmp/xxx_pk.PEM"), "path of private key.");

static void SSL_error_info(int ret, const std::string& str) {
    switch(ret) {
        case SSL_ERROR_ZERO_RETURN:
            LOG_INFO << str
                << "No more data can be read!\n";
            break;
        case SSL_ERROR_WANT_READ:
            LOG_INFO << str
                << "The operation did not complete and can be retried later.\n";
            break;
        case SSL_ERROR_WANT_X509_LOOKUP:
            LOG_INFO << str
                << "SSL_ERROR_WANT_X509_LOOKUP\n";
            break;
        case SSL_ERROR_WANT_ASYNC:
            LOG_INFO << str
                << "SSL_ERROR_WANT_ASYNC\n";
            break;
        case SSL_ERROR_WANT_ASYNC_JOB:
            LOG_INFO << str
                << "SSL_ERROR_WANT_ASYNC_JOB\n";
            break;
        case SSL_ERROR_WANT_CLIENT_HELLO_CB:
            LOG_INFO << str
                << "SSL_ERROR_WANT_CLIENT_HELLO_CB\n";
            break;
        case SSL_ERROR_SYSCALL:
            LOG_INFO << str
                << "SSL_ERROR_SYSCALL\n"
                << "errno = " << errno
                << ", strerror: " << ::strerror(errno);
            break;
        case SSL_ERROR_SSL:
            LOG_INFO << str
                << "SSL_ERROR_SSL\n"
                << ERR_error_string(ERR_get_error(), NULL) << "\n";
            break;
        case SSL_ERROR_NONE:
            LOG_INFO << str
                << "The TLS/SSL I/O operation completed.\n";
            break;
        default:
            LOG_WARN << str
                << "SSL_read is not success!\n";
    }
}

SslSocketImpl::~SslSocketImpl() {
    if (_ssl) {
        SSL_shutdown(_ssl);
        SSL_free(_ssl);
        _ssl = NULL;
    }
    if (_ssl_ctx) {
        SSL_CTX_free(_ssl_ctx);
        _ssl_ctx = NULL;
    }
}

// void SslSocketImpl::init(const SslSocketImpl& rhs) {
//     _ssl_ctx = rhs._ssl_ctx;
//     _ssl = rhs._ssl;
// }

void SslSocketImpl::init(SSL_CTX* ctx, SSL* ssl) {
    _ssl_ctx = ctx;
    _ssl = ssl;
}

int SslSocketImpl::ssl_init() {
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


void SslSocketImpl::connect(int fd) {
    _ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (_ssl_ctx == NULL) {
        LOG_ERROR << "SSL_CTX_new error!\n";
        return;
    }

    _ssl = SSL_new(_ssl_ctx);
    if (_ssl == NULL) {
        LOG_ERROR << "SSL_new error!\n";
        return;
    }

    SSL_set_fd(_ssl, fd);

    SSL_connect(_ssl);
}

void SslSocketImpl::listen() {
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

SSL* SslSocketImpl::accept(int fd) {
    SSL* ssl = SSL_new(_ssl_ctx);
    if (ssl == NULL) {
        LOG_ERROR << "SSL_new error!\n";
        return NULL;
    }
    SSL_set_fd(ssl, fd);
    int ret = SSL_accept(ssl);
    if (ret == -1) {
        SSL_error_info(SSL_get_error(ssl, ret), "SslSocketImpl::accept is not success\n");
        return NULL;
    }
    return ssl;
}

size_t SslSocketImpl::send(const std::string& msg) {
    int ret = SSL_write(_ssl, msg.data(), msg.size());
    if (ret <= 0) {
        SSL_error_info(SSL_get_error(_ssl, ret), "SslSocketImpl::send is not success!\n");
    }
    return ret;
}

size_t SslSocketImpl::send(const char* msg) {
    int ret = SSL_write(_ssl, msg, ::strlen(msg));
    if (ret <= 0) {
        SSL_error_info(SSL_get_error(_ssl, ret), "SslSocketImpl::send is not success!\n");
    }
    return ret;
};

int SslSocketImpl::recv(char* buf, size_t len) {
    int ret = SSL_read(_ssl, buf, len);
    if (ret <= 0) {
        SSL_error_info(SSL_get_error(_ssl, ret),"SslSocketImpl::recv is not success!\n");
    }
    return ret;
}
