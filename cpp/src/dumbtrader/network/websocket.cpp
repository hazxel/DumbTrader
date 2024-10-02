#include "dumbtrader/network/websocket.h"

#include <string>

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
} // namespace dumbtrader::network::detail
} // namespace dumbtrader::network