#include "dumbtrader/network/websocket.h"
// #include "dumbtrader/utils/error.h"

#include <cstring>
#include <iostream>

using namespace dumbtrader::network;

constexpr const char HOST_NAME[] = "ws.okx.com";
constexpr const int HOST_PORT = 8443;
constexpr const char SERVICE_PATH[] = "/ws/v5/business";
constexpr const char SUBSCRIBE_MSG[] = R"({"op": "subscribe", "args": [{"channel": "trades-all","instId": "ETH-USDT-SWAP"}]})";

int main() {
    WebSocketSecureClient client;
    client.init(HOST_NAME, HOST_PORT);
    client.connectService(SERVICE_PATH);
    client.send(SUBSCRIBE_MSG);
    client.recv();
    client.recv();
    client.recv();
    client.recv();

    return 0;
}