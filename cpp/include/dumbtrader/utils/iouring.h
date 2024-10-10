#ifdef LIBURING_ENABLED

#ifndef DUMBTRADER_UTILS_IOURING_H_
#define DUMBTRADER_UTILS_IOURING_H_

#include <liburing.h>

namespace dumbtrader::utils::iouring {

constexpr const char* FMT_IOURING_QUEUE_INIT_FAILED = "Failed to init iouring queue, errno: {} ({})";
constexpr const char* FMT_IOURING_IO_READ_FAILED = "Failed to execute I/O read, errno: {} ({})";
constexpr const char* FMT_IOURING_IO_SEND_FAILED = "Failed to execute I/O send, errno: {} ({})";
constexpr const char* MSG_IOURING_GET_SQE_FAILED = "Failed to get SQE (maybe no SQE available, consider increase queue depth).";


template<int QUEUE_DEPTH = 8>
class IoUring {
public:
    IoUring() {
        if (::io_uring_queue_init(QUEUE_DEPTH, &ring_, 0) < 0) {
            THROW_CERROR(FMT_IOURING_QUEUE_INIT_FAILED);
        }
    }

    ~IoUring() {
        ::io_uring_queue_exit(&ring_);
    }

    inline int read(int fd, void *dst, size_t len) {
        io_uring_sqe* sqe = ::io_uring_get_sqe(&ring_);
        if (!sqe) {
            throw std::runtime_error(MSG_IOURING_GET_SQE_FAILED);
        }

        // offset needed when reading disk files
        // no need to calculate offset when reading socket
        ::io_uring_prep_read(sqe, fd, dst, len, 0);
        ::io_uring_submit(&ring_);

        io_uring_cqe* cqe;
        ::io_uring_wait_cqe(&ring_, &cqe);
        int readBytes = cqe->res;
        if (readBytes < 0) {
            THROW_CERROR_WITH_ERRNO(FMT_IOURING_IO_READ_FAILED, -readBytes);
        }
        
        ::io_uring_cqe_seen(&ring_, cqe);
        return readBytes;
    }

    inline int write(int fd, const void* src, int len) {
        io_uring_sqe *sqe = ::io_uring_get_sqe(&ring_);
        if (!sqe) {
            throw std::runtime_error(MSG_IOURING_GET_SQE_FAILED);
        }

        ::io_uring_prep_send(sqe, fd, src, len, 0);
        ::io_uring_submit(&ring_);

        struct io_uring_cqe *cqe;
        ::io_uring_wait_cqe(&ring_, &cqe);
        int sendBytes = cqe->res;
        if (sendBytes < 0) {
            THROW_CERROR_WITH_ERRNO(FMT_IOURING_IO_SEND_FAILED, -sendBytes);
        }

        ::io_uring_cqe_seen(&ring_, cqe);
        return sendBytes;
    }

private:
    io_uring ring_;
};

} // namespace dumbtrader::utils::iouring

#endif // #define DUMBTRADER_UTILS_IOURING_H_

#endif // #ifdef LIBURING_ENABLED

