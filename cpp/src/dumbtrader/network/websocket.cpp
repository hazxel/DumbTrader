#include "dumbtrader/network/websocket.h"

#include "dumbtrader/utils/error.h"
#include "dumbtrader/utils/openssl.h"
#include "dumbtrader/network/socket.h"

#include <cstring>
#include <random>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace dumbtrader::network {

namespace detail {

#if __cplusplus >= 202002L && __has_include(<format>)
#include <format>
std::string buildServiceRequestMsg(const char* hostName, const char* servicePath) {
    return std::format(
        "GET {} HTTP/1.1\r\n"
        "Host: {}\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n",
        servicePath,
        hostName
    );
}
#else // if __cplusplus < 202002L || !__has_include(<format>)
#include <sstream>
std::string buildServiceRequestMsg(const char* hostName, const char* servicePath) {
    std::ostringstream requestStream;
    requestStream << "GET " << servicePath << " HTTP/1.1\r\n"
                  << "Host: " << hostName << "\r\n"
                  << "Upgrade: websocket\r\n"
                  << "Connection: Upgrade\r\n"
                  << "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                  << "Sec-WebSocket-Version: 13\r\n\r\n";
    return requestStream.str();
}
#endif // #if __cplusplus >= 202002L && __has_include(<format>)

void genRandMask(unsigned char (&mask)[4]) {
    std::uint32_t randomValue = std::rand(); // Generate a random 32-bit number
    std::memcpy(mask, &randomValue, 4);    
}

} // namespace dumbtrader::network::detail

WebSocketSecureClient::WebSocketSecureClient() : socket_(), ssl_ctx_(nullptr), ssl_(nullptr) {
    ::SSL_library_init();
    ::SSL_load_error_strings();
    ::OpenSSL_add_ssl_algorithms();

    ssl_ctx_ = ::SSL_CTX_new(::TLS_client_method());
    if (ssl_ctx_ == nullptr) {
        throw dumbtrader::utils::openssl::getException();
    }
    ::SSL_CTX_set_min_proto_version(ssl_ctx_, TLS1_3_VERSION);
    ::SSL_CTX_set_options(ssl_ctx_, SSL_OP_SINGLE_DH_USE);
    /* usually only server provide cert */
    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT|SSL_VERIFY_CLIENT_ONCE, 0);
    // SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, nullptr);
    // int use_cert = SSL_CTX_use_certificate_file(ctx, ".../certificate.crt" , SSL_FILETYPE_PEM);
    // int use_prv = SSL_CTX_use_PrivateKey_file(ctx, "...s/private.key", SSL_FILETYPE_PEM);
    // if (!SSL_CTX_check_private_key(ctx)) { std::cerr << "Private key does not match public certificate\n"; }
    ssl_ = ::SSL_new(ssl_ctx_);

    std::random_device rd;
    std::srand(rd());
}

void WebSocketSecureClient::connectService(const char *hostName, int port, const char* servicePath) {
    socket_.connectByHostname(hostName, port);
    int sockfd = socket_.get_fd();
    ::SSL_set_fd(ssl_, sockfd);
    ::SSL_set_connect_state(ssl_);
    for (;;) {
        int success = ::SSL_connect(ssl_);
        if(success == 1) {
            break;
        }
        unsigned long errorCode = ::SSL_get_error(ssl_, success);
        if (errorCode == SSL_ERROR_WANT_READ 
            || errorCode == SSL_ERROR_WANT_WRITE
            || errorCode == SSL_ERROR_WANT_X509_LOOKUP) {
            dumbtrader::utils::openssl::logNonFatalError(errorCode);
        // } else if (err == SSL_ERROR_ZERO_RETURN) {
        //     std::cerr << "SSL_connect: close notify received from peer" << std::endl;
        } else {
            throw dumbtrader::utils::openssl::getException(errorCode);
        }
    }
    std::string request = detail::buildServiceRequestMsg(hostName, servicePath);
    ::SSL_write(ssl_, request.c_str(), request.size());

    // ws established response - TODO: verify sec-websocket-accept 
    char buffer[4096];
    int bytes = ::SSL_read(ssl_, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    } else {
        THROW_CERROR("SSL_read failed, errno: {} ({})");
    }
}

// client must mask all frames sent to the server.
void WebSocketSecureClient::send(const std::string& message) {
    std::vector<unsigned char> frame;
    uint64_t payloadSize = message.size();
    if (payloadSize < PAYLOAD_LEN_TWO_BYTES) {
        frame.reserve(6 + payloadSize); // 2 + 4
        frame.push_back(FIN_BIT | OP_TEXT_FRAME);
        frame.push_back(MASK_BIT | payloadSize);
    } else if (payloadSize <= 0xFFFF) {
        frame.reserve(8 + payloadSize); // 2 + 2 + 4
        frame.push_back(FIN_BIT | OP_TEXT_FRAME);
        frame.push_back(MASK_BIT | PAYLOAD_LEN_TWO_BYTES);
        frame.push_back((payloadSize >> 8) & 0xFF);
        frame.push_back(payloadSize & 0xFF);
    } else {
        frame.reserve(14 + payloadSize); // 2 + 8 + 4
        frame.push_back(FIN_BIT | OP_TEXT_FRAME);
        frame.push_back(MASK_BIT | PAYLOAD_LEN_EIGHT_BYTES);
        frame.resize(10); // 2 + 8
        std::memcpy(frame.data() + 2, &payloadSize, sizeof(payloadSize));
    }
    unsigned char mask[4];
    detail::genRandMask(mask);
    frame.insert(frame.end(), std::begin(mask), std::end(mask));
    for (size_t i = 0; i < payloadSize; ++i) {
        frame.push_back(message[i] ^ mask[i % 4]);
    }
    ::SSL_write(ssl_, frame.data(), frame.size());
}

// server never mask frames sent to the client.
unsigned char WebSocketSecureClient::recv(std::string& message) {
    unsigned char twoBytes[2];
    ::SSL_read(ssl_, twoBytes, 2);
    bool isFin = (twoBytes[0] & FIN_BIT) != 0;
    unsigned char op = twoBytes[0] & OP_BITS;
    if (twoBytes[1] & MASK_BIT) {
        throw std::runtime_error("server never mask frames sent to the client.");
    }
    uint64_t payloadSize = twoBytes[1] & PAYLOAD_LEN_BITS;
    
    if (payloadSize == PAYLOAD_LEN_TWO_BYTES) {
        ::SSL_read(ssl_, twoBytes, 2);
        payloadSize = (static_cast<uint64_t>(twoBytes[0]) << 8) | static_cast<uint64_t>(twoBytes[1]);
    } else if (payloadSize == PAYLOAD_LEN_EIGHT_BYTES) {
        unsigned char extendedPayloadSize[8];
        ::SSL_read(ssl_, extendedPayloadSize, 8);
        payloadSize = (static_cast<uint64_t>(extendedPayloadSize[0]) << 56) |
                    (static_cast<uint64_t>(extendedPayloadSize[1]) << 48) |
                    (static_cast<uint64_t>(extendedPayloadSize[2]) << 40) |
                    (static_cast<uint64_t>(extendedPayloadSize[3]) << 32) |
                    (static_cast<uint64_t>(extendedPayloadSize[4]) << 24) |
                    (static_cast<uint64_t>(extendedPayloadSize[5]) << 16) |
                    (static_cast<uint64_t>(extendedPayloadSize[6]) << 8)  |
                    (static_cast<uint64_t>(extendedPayloadSize[7]));
    }
    size_t prevSize = message.size();
    message.resize(prevSize + payloadSize);
    int bytes_read = 0;
    while (payloadSize > 0) {
        bytes_read = ::SSL_read(ssl_, message.data() + prevSize, payloadSize);
        if (bytes_read == 0) {
            throw std::runtime_error("server closed.");
        } else if (bytes_read < 0) {
            THROW_CERROR("Socket SSL read error, errno: {} ({})");
        }
        payloadSize -= bytes_read;
        prevSize += bytes_read;
    }
    return isFin ? op : recv(message);
}

void WebSocketSecureClient::close() {
    if (ssl_ != nullptr) {
        ::SSL_free(ssl_);
        ssl_ = nullptr;
    }
    if (ssl_ctx_ != nullptr) {
        ::SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }
}

} // namespace dumbtrader::network