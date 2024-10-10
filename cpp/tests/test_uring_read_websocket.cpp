#include "dumbtrader/network/websocket.h"
#include "dumbtrader/network/openssl.h"

#include <chrono>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>


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

constexpr const int BUFFER_SIZE = 4096;
constexpr const int QUEUE_DEPTH = 8;

int64_t get_current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

int main() {
    WebSocketSecureClient<SSLIoUringClient> wsc;
    wsc.connectService(HOST_NAME, HOST_PORT, SERVICE_PATH);
    wsc.send(SUBSCRIBE_MSG);
    
    std::string msg;
    for (int i = 0; i < 5; ++i) {
        wsc.recv(msg);
        std::cout << " - msg No." << i << " : \n" << msg << "\n";
        msg.clear();
        std::cout << "ts: " << get_current_timestamp_ms() << "\n";
    }
    return 0;
}