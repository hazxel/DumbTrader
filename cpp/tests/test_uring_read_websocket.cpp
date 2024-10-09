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

#ifdef LIBURING_ENABLED

#include <liburing.h>

void liburing_read_ws(int fd) {
    io_uring ring;
    ::io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    char buffer[BUFFER_SIZE];
    int offset = 0;

    io_uring_sqe* sqe;
    io_uring_cqe* cqe;

    for (;;) {
        std::cout << "Preparing read." << std::endl;
        sqe = ::io_uring_get_sqe(&ring);
        if (!sqe) {
            std::cerr << "Failed to get sqe" << std::endl;
            break;
        }
        ::io_uring_prep_read(sqe, fd, buffer, sizeof(buffer), offset);
        ::io_uring_submit(&ring);

        std::cout << "Before wait." << std::endl;
        ::io_uring_wait_cqe(&ring, &cqe);
        if (cqe->res < 0) {
            std::cerr << "Read failed" << std::endl;
        } else if (cqe->res == 0) {
            std::cout << "End of file reached." << std::endl;
            break;
        } else {
            std::cout << "Read " << cqe->res << " bytes" << std::endl;
            std::cout.write(buffer, cqe->res);
            offset += cqe->res;
        }   

        ::io_uring_cqe_seen(&ring, cqe);
        std::cout << "Seen sent." << std::endl;
    }
    ::io_uring_queue_exit(&ring);
    ::close(fd);
}
#else
void liburing_read_ws(int fd) { std::cout << "I/O Uring not supported on this platform." << std::endl; }
#endif

int main() {
    WebSocketSecureClient<SSLMemoryBioClient> wsc;

    wsc.connectService(HOST_NAME, HOST_PORT, SERVICE_PATH);
    wsc.send(SUBSCRIBE_MSG);

    int fd = wsc.getSockFd();
    // liburing_read_ws(fd);

    std::string msg;
    for (int i = 0; i < 5; ++i) {
        wsc.recv(msg);
        std::cout << " - msg No." << i << " : \n" << msg << "\n";
        msg.clear();
        std::cout << "ts: " << get_current_timestamp_ms() << "\n";
    }
    return 0;
}