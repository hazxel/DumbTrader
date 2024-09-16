#ifndef DUMBTRADER_NETWORK_WEBSOCKET_H_
#define DUMBTRADER_NETWORK_WEBSOCKET_H_

#include "dumbtrader/utils/error.h"
#include "dumbtrader/utils/openssl.h"
#include "dumbtrader/network/socket.h"

#include <cstring>      // std::memset

#include <arpa/inet.h>  // inet_addr (convert `char*` ip addr to `__uint32_t`)
#include <fcntl.h>      // fnctl
#include <netdb.h>      // addrinfo, getaddrinfo
#include <netinet/in.h> // sockaddr_in (specific descriptor for IPv4 communication)
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h> // sockaddr (generic descriptor for any socket operation), connect, listen, bind, accept
#include <sys/types.h>
#include <unistd.h>     // close

namespace dumbtrader::network {

// constexpr const char* FMT_SEM_CREATE_FAILED = "Failed to create semaphore {}, errno: {} ({})";
class WebSocketSecureClient {
public:
    WebSocketSecureClient() : socket_(), ssl_ctx_(nullptr), ssl_(nullptr) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        ssl_ctx_ = SSL_CTX_new(TLS_client_method());
        if (ssl_ctx_ == NULL) {
            throw dumbtrader::utils::openssl::getOpenSSLError();
        }

        SSL_CTX_set_min_proto_version(ssl_ctx_, TLS1_3_VERSION);
        SSL_CTX_set_options(ssl_ctx_, SSL_OP_SINGLE_DH_USE);

        // int use_cert = SSL_CTX_use_certificate_file(ctx, "/Users/boyan/Documents/playground/DumbTrader/credentials/certificate.crt" , SSL_FILETYPE_PEM);
        // int use_prv = SSL_CTX_use_PrivateKey_file(ctx, "/Users/boyan/Documents/playground/DumbTrader/credentials/private.key", SSL_FILETYPE_PEM);
        // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT|SSL_VERIFY_CLIENT_ONCE, 0);
        // if (!SSL_CTX_check_private_key(ctx)) {
        //     std::cerr << "Private key does not match the public certificate" << std::endl;
        // }

        ssl_ = SSL_new(ssl_ctx_);
    }

    ~WebSocketSecureClient() {
        if (ssl_ != nullptr) {
            SSL_free(ssl_);
            ssl_ = nullptr;
        }
        if (ssl_ctx_ != nullptr) {
            SSL_CTX_free(ssl_ctx_);
            ssl_ctx_ = nullptr;
        }
    }

    void init(const char *host, int port) {
        socket_.connectByHostname(host, port);
        int sockfd = socket_.get_fd();
        SSL_set_fd(ssl_, sockfd);
        SSL_set_connect_state(ssl_);

        for (;;) {
            int success = SSL_connect(ssl_);
            if(success == 1) {
                break;
            }

            unsigned long errorCode = SSL_get_error(ssl_, success);
            if (errorCode == SSL_ERROR_WANT_READ 
                || errorCode == SSL_ERROR_WANT_WRITE
                || errorCode == SSL_ERROR_WANT_X509_LOOKUP) {
                std::cout << "continue" << std::endl;
                continue;
            // } else if (err == SSL_ERROR_ZERO_RETURN) {
            //     printf("SSL_connect: close notify received from peer");
            //     exit(18);
            } else {
                throw dumbtrader::utils::openssl::getOpenSSLError(errorCode);
            }
        }
    }

    void connect() { // don't need mask
        const std::string request = 
        "GET /ws/v5/business HTTP/1.1\r\n" // or /ws/v5/public for some channels
        "Host: ws.okx.com\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
        SSL_write(ssl_, request.c_str(), request.size());
    }

    void send() {
        std::string message = R"({"op": "subscribe", "args": [{"channel": "trades-all","instId": "ETH-USDT-SWAP"}]})";
        std::vector<unsigned char> frame;
        
        // Create the WebSocket frame header
        unsigned char byte1 = 0x81; // FIN + Text frame
        frame.push_back(byte1);
        
        size_t payload_len = message.size();
        
        // Mask bit is 1 (client-to-server), and we handle small payloads (<126 bytes)
        if (payload_len <= 125) {
            frame.push_back(0x80 | payload_len); // 0x80 sets the mask bit
        } else if (payload_len <= 65535) {
            frame.push_back(0x80 | 126); // 126 means 2-byte payload length follows
            frame.push_back((payload_len >> 8) & 0xFF); // Most significant byte
            frame.push_back(payload_len & 0xFF);        // Least significant byte
        } else {
            // Handle larger payloads if needed
            frame.push_back(0x80 | 127); // 127 means 8-byte payload length follows
            for (int i = 7; i >= 0; --i) {
                frame.push_back((payload_len >> (8 * i)) & 0xFF);
            }
        }
        
        // Generate a masking key (4 bytes)
        unsigned char mask[4] = {0x12, 0x34, 0x56, 0x78}; // Example mask key (randomly generated in real scenarios)
        frame.insert(frame.end(), std::begin(mask), std::end(mask));
        
        // Mask the payload data
        for (size_t i = 0; i < payload_len; ++i) {
            frame.push_back(message[i] ^ mask[i % 4]);
        }

        SSL_write(ssl_, frame.data(), frame.size());

        // subscribe response
        char buffer[4096];
        int bytes = SSL_read(ssl_, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << "Received: " << buffer << std::endl;
        } else {
            THROW_CERROR("SSL_read failed, errno: {} ({})");
        }

    }

    void recv() {
        unsigned char buffer[2];
        
        // Read the first 2 bytes of the WebSocket frame (fin, opcode, mask, payload length)
        SSL_read(ssl_, buffer, 2);
        
        bool fin = (buffer[0] & 0x80) != 0;
        unsigned char opcode = buffer[0] & 0x0F;
        bool is_masked = (buffer[1] & 0x80) != 0;
        size_t payload_len = buffer[1] & 0x7F;
        
        // Handle extended payload lengths (126 or 127)
        if (payload_len == 126) {
            unsigned char extended_payload[2];
            SSL_read(ssl_, extended_payload, 2);
            payload_len = (extended_payload[0] << 8) | extended_payload[1];
        } else if (payload_len == 127) {
            unsigned char extended_payload[8];
            SSL_read(ssl_, extended_payload, 8);
            // Handle 8-byte extended payload length
            // For simplicity, we'll assume it's small and only use the lower 4 bytes
            payload_len = (extended_payload[4] << 24) | (extended_payload[5] << 16) |
                        (extended_payload[6] << 8) | extended_payload[7];
        }
        
        // Read the payload data
        std::vector<unsigned char> payload(payload_len);
        SSL_read(ssl_, payload.data(), payload_len);

        // Print the payload
        if (opcode == 0x1) {
            // Text frame
            std::string message(payload.begin(), payload.end());
            std::cout << "Received message: " << message << std::endl;
        } else if (opcode == 0x2) {
            // Binary frame
            std::cout << "Received binary data" << std::endl;
        } else {
            std::cout << "Received unknown opcode: " << opcode << std::endl;
        }
    }

    void close();

private: 
    Socket<Side::CLIENT, Mode::BLOCK> socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
};

} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_WEBSOCKET_H_
