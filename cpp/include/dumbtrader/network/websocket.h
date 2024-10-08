#ifndef DUMBTRADER_NETWORK_WEBSOCKET_H_
#define DUMBTRADER_NETWORK_WEBSOCKET_H_

#include "dumbtrader/network/socket.h"
#include "dumbtrader/network/openssl.h"

#include <cstring>
#include <string>
#include <random>

namespace dumbtrader::network {

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

namespace detail {
std::string buildServiceRequestMsg(const char* hostName, const char* servicePath);

inline void genRandMask(unsigned char (&mask)[4]) {
    std::uint32_t randomValue = std::rand(); // Generate a random 32-bit number
    std::memcpy(mask, &randomValue, 4);    
}
} // namespace dumbtrader::network::detail

template <typename SSLClientType = SSLDirectSocketClient>
class WebSocketSecureClient {
    public:
    WebSocketSecureClient() {
        std::random_device rd;
        std::srand(rd());
    }

    void connectService(const char *hostName, int port, const char* servicePath) {
        sslClient_.connect(hostName, port);
        std::string request = detail::buildServiceRequestMsg(hostName, servicePath);
        sslClient_.write(request.c_str(), request.size());

        // ws established response - TODO: verify sec-websocket-accept 
        char buffer[4096];
        sslClient_.read(buffer, sizeof(buffer) - 1);
        buffer[4095] = '\0';
        std::cout << "Handshake response received: " << buffer << std::endl;
    }

    void send(const std::string& message) {
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
        sslClient_.write(frame.data(), frame.size());
    }

    void recv(std::string& message) {
        unsigned char twoBytes[2];
        sslClient_.read(twoBytes, 2);
        bool isFin = (twoBytes[0] & FIN_BIT) != 0;
        unsigned char op = twoBytes[0] & OP_BITS;
        if (twoBytes[1] & MASK_BIT) {
            throw std::runtime_error("server never mask frames sent to the client.");
        }
        uint64_t payloadSize = twoBytes[1] & PAYLOAD_LEN_BITS;
        
        if (payloadSize == PAYLOAD_LEN_TWO_BYTES) {
            sslClient_.read(twoBytes, 2);
            payloadSize = (static_cast<uint64_t>(twoBytes[0]) << 8) | static_cast<uint64_t>(twoBytes[1]);
        } else if (payloadSize == PAYLOAD_LEN_EIGHT_BYTES) {
            unsigned char extendedPayloadSize[8];
            sslClient_.read(extendedPayloadSize, 8);
            payloadSize = (static_cast<uint64_t>(extendedPayloadSize[0]) << 56) |
                        (static_cast<uint64_t>(extendedPayloadSize[1]) << 48) |
                        (static_cast<uint64_t>(extendedPayloadSize[2]) << 40) |
                        (static_cast<uint64_t>(extendedPayloadSize[3]) << 32) |
                        (static_cast<uint64_t>(extendedPayloadSize[4]) << 24) |
                        (static_cast<uint64_t>(extendedPayloadSize[5]) << 16) |
                        (static_cast<uint64_t>(extendedPayloadSize[6]) << 8)  |
                        (static_cast<uint64_t>(extendedPayloadSize[7]));
        }
        message.resize(payloadSize);
        sslClient_.read(message.data(), payloadSize);
    }

    int getSockFd() const { return sslClient_.getSocketFd(); }
private: 
    SSLClientType sslClient_;
};
} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_WEBSOCKET_H_
