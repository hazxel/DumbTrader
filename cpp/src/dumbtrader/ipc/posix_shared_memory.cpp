#include "dumbtrader/ipc/posix_shared_memory.h"

#include <stdexcept>

#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, O_RDWR, ...)
#include <unistd.h>
#include <cstring>      // strcpy


namespace dumbtrader{

const mode_t PosixSharedMemory::SHM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // 666

PosixSharedMemory::PosixSharedMemory(const std::string& name, size_t size, bool create)
    : shmName_(nullptr), size_(size), shm_fd_(-1), ptr_(nullptr) {
    shmName_ = new char[name.size() + 1]; // +1 for '\0'
    std::strcpy(shmName_, name.c_str());
    if (create) {
        shm_fd_ = shm_open(shmName_, O_CREAT | O_RDWR, SHM_PERM_MODE);
        if (shm_fd_ == -1) {
            throw std::runtime_error("Failed to create shared memory");
        }
        if (ftruncate(shm_fd_, size_) == -1) {
            shm_unlink(shmName_);
            throw std::runtime_error("Failed to set size for shared memory");
        }
    } else {
        shm_fd_ = shm_open(shmName_, O_RDWR, SHM_PERM_MODE);
        if (shm_fd_ == -1) {
            throw std::runtime_error("Failed to open shared memory");
        }
    }

    ptr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (ptr_ == MAP_FAILED) {
        shm_unlink(shmName_);
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
    shm_unlink(shmName_);
    delete[] shmName_;
}

void* PosixSharedMemory::get() const {
    return ptr_;
}

size_t PosixSharedMemory::size() const {
    return size_;
}

} // namespace dumbtrader
