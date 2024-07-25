#include "dumbtrader/ipc/posix_shared_memory.h"

namespace dumbtrader{

PosixSharedMemory::PosixSharedMemory(const std::string& name, size_t size, bool create)
    : name_(name), size_(size), shm_fd_(-1), ptr_(nullptr) {
    if (create) {
        shm_fd_ = shm_open(name_.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error("Failed to create shared memory");
        }
        if (ftruncate(shm_fd_, size_) == -1) {
            shm_unlink(name_.c_str());
            throw std::runtime_error("Failed to set size for shared memory");
        }
    } else {
        shm_fd_ = shm_open(name_.c_str(), O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error("Failed to open shared memory");
        }
    }

    ptr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (ptr_ == MAP_FAILED) {
        shm_unlink(name_.c_str());
        throw std::runtime_error("Failed to map shared memory");
    }
}

PosixSharedMemory::~PosixSharedMemory() {
    if (ptr_) {
        munmap(ptr_, size_);
    }
    if (shm_fd_ != -1) {
        close(shm_fd_);
    }
    shm_unlink(name_.c_str());
}

void* PosixSharedMemory::get() const {
    return ptr_;
}

size_t PosixSharedMemory::size() const {
    return size_;
}

} // namespace dumbtrader
