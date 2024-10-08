#include "dumbtrader/network/websocket.h"

#include <iostream>
#include <chrono>

using namespace dumbtrader::network;

// biance: wss://ws-api.binance.com:443/ws-api/v3/xxx ??? 
constexpr const char HOST_NAME[] = "fstream.binance.com";
constexpr const int HOST_PORT = 443;
constexpr const char SERVICE_PATH[] = "/ws";
constexpr const char SUBSCRIBE_MSG[] = R"({"method": "SUBSCRIBE","params": ["btcusdt@depth"],"id": 1})";

// okx: wss://ws.okx.com:8443/ws/v5/xxx
// constexpr const char HOST_NAME[] = "ws.okx.com";
// constexpr const int HOST_PORT = 8443;
// constexpr const char SERVICE_PATH[] = "/ws/v5/public";
// constexpr const char SUBSCRIBE_MSG[] = R"({"op": "subscribe", "args": [{"channel": "books","instId": "ETH-USDT-SWAP"}]})";

int64_t get_current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

int main() {
    // WebSocketSecureClient<SSLMemoryBioClient> client;
    WebSocketSecureClient<SSLSocketBioClient> client;
    client.connectService(HOST_NAME, HOST_PORT, SERVICE_PATH);
    client.send(SUBSCRIBE_MSG);

    std::string msg;
    for (int i = 0; i < 5; ++i) {
        client.recv(msg);
        std::cout << " - msg No." << i << " : \n" << msg << "\n";
        msg.clear();
        std::cout << "ts: " << get_current_timestamp_ms() << "\n";
    }
    return 0;
}