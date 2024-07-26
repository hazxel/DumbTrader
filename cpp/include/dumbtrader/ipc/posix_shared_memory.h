#ifndef DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
#define DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_

#include <stdexcept>
#include <string>

#include <cstring>      // strcpy
#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, O_RDWR, ...)
#include <sys/mman.h>   // shared memory
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...), mode_t
#include <unistd.h>     // ftruncate


namespace dumbtrader{

template <bool IsOwner = true>
class PosixSharedMemory {
public:
    PosixSharedMemory(const std::string& name, size_t size)
        : shmName_(nullptr), size_(size), shm_fd_(-1), ptr_(nullptr) {
        shmName_ = new char[name.size() + 1]; // +1 for '\0'
        std::strcpy(shmName_, name.c_str());
        if constexpr (IsOwner) {
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

    ~PosixSharedMemory() {
        if (ptr_) {
            munmap(ptr_, size_);
        }
        if (shm_fd_ != -1) {
            close(shm_fd_);
        }
        shm_unlink(shmName_);
        delete[] shmName_;
    }

    void* get() const {
        return ptr_;
    }

    size_t size() const {
        return size_;
    }

    static const mode_t SHM_PERM_MODE;

private:
    char *shmName_;
    size_t size_;
    int shm_fd_;
    void* ptr_;
};

template<bool IsOwner>
const mode_t PosixSharedMemory<IsOwner>::SHM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // 666

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
