#ifdef LIBURING_ENABLED

#ifndef DUMBTRADER_UTILS_IOURING_H_
#define DUMBTRADER_UTILS_IOURING_H_

#include <liburing.h>

namespace dumbtrader::utils::iouring {

template<int QUEUE_DEPTH = 8>
class IoUring {
public:
    IoUring() {
        ::io_uring_queue_init(QUEUE_DEPTH, &ring_, 0);
    }

    ~IoUring() {
        ::io_uring_queue_exit(&ring_);
    }

    int read(int fd, void *dst, size_t len) {
        io_uring_cqe* cqe;
        io_uring_sqe* sqe = ::io_uring_get_sqe(&ring_);
        if (!sqe) {
            std::cerr << "Failed to get sqe" << std::endl;
            break;
        }

        // no need to calculate offset when reading socket
        ::io_uring_prep_read(sqe, fd, dst, len, 0);
        ::io_uring_submit(&ring_);

        std::cout << "Before wait." << std::endl;
        ::io_uring_wait_cqe(&ring_, &cqe);
        if (cqe->res < 0) {
            std::cerr << "Read failed" << std::endl;
            return -1;
        } else {
            std::cout << "Read " << cqe->res << " bytes" << std::endl;
            // std::cout.write(dst, cqe->res);
            return cqe->res;
        }   

        ::io_uring_cqe_seen(&ring_, cqe);
        std::cout << "Seen sent." << std::endl;
    }

private:
    io_uring ring_;
    // char buffer_[BUFFER_SIZE];
};

} // namespace dumbtrader::utils::iouring

#endif // #define DUMBTRADER_UTILS_IOURING_H_

#endif // #ifdef LIBURING_ENABLED

