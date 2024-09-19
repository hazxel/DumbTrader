#ifndef DUMBTRADER_NETWORK_WEBSOCKET_H_
#define DUMBTRADER_NETWORK_WEBSOCKET_H_

#include "dumbtrader/network/socket.h"

#include <string>

#include <openssl/ossl_typ.h>

namespace dumbtrader::network {

namespace detail {
std::string buildServiceRequestMsg(const char* hostName, const char* servicePath);
void genRandMask(unsigned char (&mask)[4]);
} // namespace dumbtrader::network::detail

constexpr unsigned char OP_CONTINUATION = 0x0;
constexpr unsigned char OP_TEXT_FRAME = 0x1;
constexpr unsigned char OP_BIN_FRAME = 0x2;
constexpr unsigned char OP_CLOSE = 0x8;
constexpr unsigned char OP_PING = 0x9;
constexpr unsigned char OP_PONG = 0xA;

constexpr unsigned char FIN_BIT = 0x80;
constexpr unsigned char OP_BITS = 0x0F;
constexpr unsigned char MASK_BIT = 0x80;
constexpr unsigned char PAYLOAD_LEN_BITS = 0x7F;
constexpr unsigned char PAYLOAD_LEN_TWO_BYTES = 0x7E; // 126, means 2-byte payload length field follows
constexpr unsigned char PAYLOAD_LEN_EIGHT_BYTES = 0x7F; // 127, means 8-byte payload length field follows
// constexpr const char* FMT_SEM_CREATE_FAILED = "Failed to create semaphore {}, errno: {} ({})";

class WebSocketSecureClient {
public:
    WebSocketSecureClient();

    ~WebSocketSecureClient() { close(); }

    void connectService(const char *hostName, int port, const char* servicePath);

    void send(const std::string& message);

    unsigned char recv(std::string& message);

    void close();
private: 
    Socket<Side::CLIENT, Mode::BLOCK> socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
};

} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_WEBSOCKET_H_
