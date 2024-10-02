#ifndef DUMBTRADER_NETWORK_OPENSSL_H_
#define DUMBTRADER_NETWORK_OPENSSL_H_

#include "dumbtrader/network/socket.h"

#include <openssl/ossl_typ.h>

#include <stdexcept>

namespace dumbtrader::network {

class OpenSSLException : public std::runtime_error {
public:
    explicit OpenSSLException(const std::string& message)
        : std::runtime_error(message) {}
};

void logOpenSSLError(unsigned long errorCode);

inline OpenSSLException getException(unsigned long errorCode);

inline OpenSSLException getException();

class SSLDirectSocketClient {

};

class SSLSocketBioClient {
public:
    using BlockSocketClient = Socket<Side::CLIENT, Mode::BLOCK>;

    SSLSocketBioClient();
    ~SSLSocketBioClient();

    void connect(const char *hostName, int port);
    int read(void *dst, size_t len);
    void write(const void* data, int length);

private:
    BlockSocketClient socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
    BIO* bio_;
};

class SSLMemoryBioClient {
public:
    using BlockSocketClient = Socket<Side::CLIENT, Mode::BLOCK>;
    static constexpr size_t BUF_SIZE = 4096;

    SSLMemoryBioClient();
    ~SSLMemoryBioClient();

    void connect(const char *hostName, int port);
    int read(void *dst, size_t len);
    int write(const void *src, size_t len);

private:
    BlockSocketClient socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
    BIO* bio_;
    void* buffer_;
};

class SSLIoUringClient {

};

} // namespace dumbtrader::network

#endif // #define DUMBTRADER_NETWORK_OPENSSL_H_