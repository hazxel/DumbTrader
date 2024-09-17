#ifndef DUMBTRADER_NETWORK_WEBSOCKET_H_
#define DUMBTRADER_NETWORK_WEBSOCKET_H_

#include "dumbtrader/network/socket.h"

#include <string>

#include <openssl/types.h>

namespace dumbtrader::network {

namespace detail {
std::string buildServiceRequestMsg(const char* hostName, const char* servicePath);
} // namespace dumbtrader::network::detail

// constexpr const char* FMT_SEM_CREATE_FAILED = "Failed to create semaphore {}, errno: {} ({})";
class WebSocketSecureClient {
public:
    WebSocketSecureClient();

    ~WebSocketSecureClient() { close(); }

    void connectService(const char *hostName, int port, const char* servicePath);

    void send(const std::string& message);

    void recv();

    void close();
private: 
    Socket<Side::CLIENT, Mode::BLOCK> socket_;
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
};

} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_WEBSOCKET_H_
