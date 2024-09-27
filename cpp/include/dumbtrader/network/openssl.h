#ifndef DUMBTRADER_NETWORK_OPENSSL_H_
#define DUMBTRADER_NETWORK_OPENSSL_H_

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace dumbtrader::network::openssl {

class OpenSSLException : public std::runtime_error {
public:
    explicit OpenSSLException(const std::string& message)
        : std::runtime_error(message) {}
};

inline void logOpenSSLError(unsigned long errorCode) {
    std::cerr << "OpenSSL " 
              << ERR_error_string(errorCode, nullptr)
              << ", reason: "
              << ERR_reason_error_string(errorCode)
              << std::endl;
}

inline OpenSSLException getException(unsigned long errorCode) {
    return OpenSSLException(ERR_error_string(errorCode, nullptr));
}

inline OpenSSLException getException() {
    unsigned long errorCode = ERR_get_error();
    if (errorCode != 0) {
        return OpenSSLException(ERR_error_string(errorCode, nullptr));
    }
    return OpenSSLException("No OpenSSL error in the stack");
}

class SSLMemoryBio {
public:
    SSLMemoryBio(SSL *ssl) : ssl_(ssl), mem_bio_(::BIO_new(BIO_s_mem())), pending_(0) {
        ::SSL_set_bio(ssl, mem_bio_, mem_bio_);
    };

    // TODO: ?? BIO *mem_bio = BIO_new_mem_buf(buf, data_len);

    inline int pending() const { return pending_; }

    inline void writeRawData(void *src, int len) {
        ::BIO_write(mem_bio_, src, len);
        pending_ += len;
    }

    // block read
    inline int readDecryptedData(void *dst, size_t len) {
        int readBytes = ::SSL_read(ssl_, dst, len);
        if (readBytes > 0) {
            pending_ -= readBytes;
            return readBytes;
        }
        int err = ::SSL_get_error(ssl_, readBytes);
        if (err == SSL_ERROR_WANT_READ) {
            return -1;
        }
        logOpenSSLError(err);
        throw dumbtrader::network::openssl::getException(err);
    }
private:
    SSL *ssl_;
    BIO *mem_bio_; // dynamic buffered
    size_t pending_;
};

template<size_t BUF_SIZE = 4096>
class SSLConnSocket {
public:
    using BlockCltSock = Socket<Side::CLIENT, Mode::BLOCK>;

    SSLConnSocket(BlockCltSock& sock, SSL *ssl) : socket_(sock), sslMemBio_(ssl) {}

    inline void readFromSock() { // block
        void* buffer = ::malloc(BUF_SIZE);
        ssize_t readBytes = socket_.recv(buffer, BUF_SIZE);
        sslMemBio_.writeRawData(buffer, readBytes);
    }
    
    inline void read(void *dst, size_t len) { // block
        for(;;) {
            int readBytes = sslMemBio_.readDecryptedData(dst, len);
            if (readBytes < 0) { readFromSock(); } else { break; }
        }
    }
private:
    SSLMemoryBio sslMemBio_;
    BlockCltSock &socket_;
};

// inline void ssl_info_callback(const SSL* ssl, int where, int ret) {
//     if (ret == 0) return;

//     if (where & SSL_CB_HANDSHAKE_START) {
//         std::cout << "SSL handshake started\n";
//     }
//     if (where & SSL_CB_HANDSHAKE_DONE) {
//         std::cout << "SSL handshake completed\n";
//     }
//     if (where & SSL_CB_ALERT) {
//         const char* alert_type = (where & SSL_CB_READ) ? "read" : "write";
//         std::cout << "SSL alert: " << alert_type << "(" << SSL_alert_desc_string_long(ret) << ")\n";
//     }
//     if (where & SSL_CB_LOOP) {
//         std::cout << "SSL state: " << SSL_state_string_long(ssl) << "\n";
//     }
// }

} // namespace dumbtrader::network::openssl

#endif // #define DUMBTRADER_NETWORK_OPENSSL_H_