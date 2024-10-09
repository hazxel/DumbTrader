#ifndef DUMBTRADER_NETWORK_OPENSSL_H_
#define DUMBTRADER_NETWORK_OPENSSL_H_

#include "dumbtrader/network/socket.h"

#include <openssl/ossl_typ.h>

#ifdef LIBURING_ENABLED
#include "dumbtrader/utils/iouring.h"
#endif

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
public:
    using BlockSocketClient = Socket<Side::CLIENT, Mode::BLOCK>;

    SSLDirectSocketClient();
    ~SSLDirectSocketClient();

    void connect(const char *hostName, int port);
    int read(void *dst, size_t len);
    void write(const void* data, int length);

    int getSocketFd() const { return socket_.getFd(); }

protected:
    BlockSocketClient socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
};

class SSLSocketBioClient : public SSLDirectSocketClient {
public:
    SSLSocketBioClient() : SSLDirectSocketClient(), bio_(nullptr) {}
    void connect(const char *hostName, int port);
private:
    BIO* bio_;
};

class SSLMemoryBioClient : public SSLDirectSocketClient {
public:
    static constexpr size_t BUF_SIZE = 4096;
    SSLMemoryBioClient() : SSLDirectSocketClient(), bio_(nullptr), buffer_(::malloc(BUF_SIZE)) {}

    ~SSLMemoryBioClient() {
        if (buffer_ != nullptr) {
            free(buffer_);
        }
    }

    void connect(const char *hostName, int port);
    int read(void *dst, size_t len);
    int write(const void *src, size_t len);

private:
    BIO* bio_;
    void* buffer_;
};

#ifdef LIBURING_ENABLED
class SSLIoUringClient : public SSLDirectSocketClient {
public:
    static constexpr size_t BUF_SIZE = 4096;
    SSLIoUringClient() : bio_(nullptr), buffer_(::malloc(BUF_SIZE)), uring_() {}

    ~SSLIoUringClient() {
        if (buffer_ != nullptr) {
            free(buffer_);
        }
    }

    void connect(const char *hostName, int port);
    int read(void *dst, size_t len);
    int write(const void *src, size_t len);

private:
    BIO* bio_;
    void* buffer_;
    dumbtrader::utils::iouring::IoUring uring_;
};
#else // #ifdef LIBURING_ENABLED
class SSLIoUringClient {
public:
    void connect(const char *hostName, int port) {}
    int read(void *dst, size_t len) { throw std::logic_error("iouring not supported on current platform."); }
    int write(const void *src, size_t len) { throw std::logic_error("iouring not supported on current platform."); }
};
#endif // #ifdef LIBURING_ENABLED

} // namespace dumbtrader::network

#endif // #define DUMBTRADER_NETWORK_OPENSSL_H_