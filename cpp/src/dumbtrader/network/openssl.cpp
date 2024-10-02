#include "dumbtrader/network/openssl.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <string>
#include <iostream>

namespace dumbtrader::network {

void logOpenSSLError(unsigned long errorCode) {
    std::cerr << "OpenSSL "
              << ERR_error_string(errorCode, nullptr)
              << ", reason: "
              << ERR_reason_error_string(errorCode)
              << std::endl;
}

OpenSSLException getException(unsigned long errorCode) {
    if (errorCode == 0) {
        return OpenSSLException("No OpenSSL error in the stack");
    }
    return OpenSSLException(ERR_error_string(errorCode, nullptr));
}

OpenSSLException getException() {
    return getException(ERR_get_error());
}


SSLDirectSocketClient::SSLDirectSocketClient()
    : ssl_ctx_(nullptr), ssl_(nullptr){
    ::SSL_library_init();
    ::SSL_load_error_strings();
    ::ERR_load_crypto_strings();
    ::OpenSSL_add_ssl_algorithms();

    ssl_ctx_ = ::SSL_CTX_new(::TLS_client_method());
    if (ssl_ctx_ == nullptr) {
        throw getException();
    }
    ::SSL_CTX_set_options(ssl_ctx_, SSL_OP_SINGLE_DH_USE);

    ssl_ = ::SSL_new(ssl_ctx_);
    if (ssl_ == nullptr) {
        throw getException();
    }
}

SSLDirectSocketClient::~SSLDirectSocketClient() {
    if (ssl_ != nullptr) {
        ::SSL_free(ssl_);
        ssl_ = nullptr;
    }
    if (ssl_ctx_ != nullptr) {
        ::SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }
}

void SSLDirectSocketClient::connect(const char *hostName, int port) {
    socket_.connectByHostname(hostName, port);
    ::SSL_set_fd(ssl_, socket_.getFd());
    ::SSL_set_connect_state(ssl_);
    for (;;) {
        int success = ::SSL_connect(ssl_);
        if (success == 1) {
            break;
        }
        unsigned long errorCode = ::SSL_get_error(ssl_, success);
        if (errorCode == SSL_ERROR_WANT_READ 
            || errorCode == SSL_ERROR_WANT_WRITE
            || errorCode == SSL_ERROR_WANT_X509_LOOKUP) {
            logOpenSSLError(errorCode);
        } else {
            logOpenSSLError(errorCode);
            throw getException(errorCode);
        }
    }
}

int SSLDirectSocketClient::read(void *dst, size_t len) {
    for (;;) {
        int readBytes = ::SSL_read(ssl_, dst, len);
        if (readBytes > 0) {
            return readBytes;
        }
        int err = ::SSL_get_error(ssl_, readBytes);
        if (err != SSL_ERROR_WANT_READ) {
            logOpenSSLError(err);
            throw getException(err);
        }
    }
}

void SSLDirectSocketClient::write(const void* data, int length) {
    int writeBytes = ::SSL_write(ssl_, data, length);
    if (writeBytes <= 0) {
        int err = ::SSL_get_error(ssl_, writeBytes);
        logOpenSSLError(err);
        throw getException(err);
    }
}

void SSLSocketBioClient::connect(const char *hostName, int port) {
    socket_.connectByHostname(hostName, port);
    bio_ = ::BIO_new(BIO_s_socket());
    BIO_set_fd(bio_, socket_.getFd(), BIO_NOCLOSE);
    ::SSL_set_bio(ssl_, bio_, bio_);
    ::SSL_set_connect_state(ssl_);
    for (;;) {
        int success = ::SSL_connect(ssl_);
        if (success == 1) {
            break;
        }
        unsigned long errorCode = ::SSL_get_error(ssl_, success);
        if (errorCode == SSL_ERROR_WANT_READ 
            || errorCode == SSL_ERROR_WANT_WRITE
            || errorCode == SSL_ERROR_WANT_X509_LOOKUP) {
            logOpenSSLError(errorCode);
        } else {
            logOpenSSLError(errorCode);
            throw getException(errorCode);
        }
    }
}

void SSLMemoryBioClient::connect(const char *hostName, int port) {
    SSLDirectSocketClient::connect(hostName, port);
    bio_ = ::BIO_new(BIO_s_mem()); // membio only take over after handshake
    ::SSL_set_bio(ssl_, bio_, bio_);
}

int SSLMemoryBioClient::read(void *dst, size_t len) {
    for (;;) {
        int readSSLBytes = ::SSL_read(ssl_, dst, len);
        if (readSSLBytes > 0) {
            return readSSLBytes;
        }
        int err = ::SSL_get_error(ssl_, readSSLBytes);
        if (err != SSL_ERROR_WANT_READ) {
            logOpenSSLError(err);
            throw getException(err);
        }
        ssize_t readSockBytes = socket_.recv(buffer_, BUF_SIZE);
        ::BIO_write(bio_, buffer_, readSockBytes);
    }
}

int SSLMemoryBioClient::write(const void *src, size_t len) {
    size_t totalWritten = 0;
    while (totalWritten < len) {
        int writtenSSLBytes = ::SSL_write(ssl_, static_cast<const char*>(src) + totalWritten, len - totalWritten);
        if (writtenSSLBytes > 0) {
            totalWritten += writtenSSLBytes;
            continue;
        }
        int err = ::SSL_get_error(ssl_, writtenSSLBytes);
        logOpenSSLError(err);
        throw getException(err);
    }
    for (;;) {
        int readBioBytes = ::BIO_read(bio_, buffer_, BUF_SIZE);
        if (readBioBytes == 0) {
            break; // no data
        } else if (readBioBytes < 0) {
            int err = ::SSL_get_error(ssl_, readBioBytes);
            if (err == SSL_ERROR_NONE) { 
                break; // no data
            } else if (err == SSL_ERROR_SYSCALL) {
                LOG_CERROR("Failed to read from ssl memory bio, maybe no data. errno: {} ({})");
                break; // maybe also no data
            }
            logOpenSSLError(err);
            throw getException(err);
        }
        socket_.send(buffer_, readBioBytes);
    }
    return totalWritten;
}

}