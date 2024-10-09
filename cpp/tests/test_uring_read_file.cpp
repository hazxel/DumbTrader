#include "dumbtrader/network/websocket.h"
#include "dumbtrader/network/openssl.h"

#include <chrono>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

constexpr const int BUFFER_SIZE = 4096;
constexpr const int QUEUE_DEPTH = 8;

int64_t get_current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

#ifdef LIBURING_ENABLED

#include <liburing.h>

void liburing_read_file() {
    io_uring ring;
    ::io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    int fd = ::open("../book.txt", O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }

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
void liburing_read_file() { std::cout << "I/O Uring not supported on this platform." << std::endl; }
#endif

int main() {
    //// raw sys call?
    // struct io_uring_params p; ？？？ syntax？
    // std::memset(&p, 0, sizeof(p));
    // ::syscall(SYS_io_uring_setup, QUEUE_DEPTH, &p);
    
    liburing_read_file();
    return 0;
}