#ifndef DUMBTRADER_NETWORK_WEBSOCKET_H_
#define DUMBTRADER_NETWORK_WEBSOCKET_H_

#include "dumbtrader/utils/error.h"

namespace dumbtrader::network {

// constexpr const char* FMT_SEM_CREATE_FAILED = "Failed to create semaphore {}, errno: {} ({})";
class WebsocketClient {
public:
    static WebsocketClient connect(const std::string &uri);

    ~WebsocketClient() {
        close();
    }

    void send();

    void recv();

    void close();

};

} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_WEBSOCKET_H_
