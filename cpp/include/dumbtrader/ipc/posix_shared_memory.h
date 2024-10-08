#ifndef DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
#define DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_

#include <dumbtrader/utils/error.h>

#include <string>

#include <cstring>      // strcpy
#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, O_RDWR, ...)
#include <sys/mman.h>   // shared memory
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...), mode_t
#include <unistd.h>     // ftruncate, close


namespace dumbtrader::ipc {

constexpr const char* FMT_SHM_CREATE_FAILED = "Failed to create shared memory {}, errno: {} ({})";
constexpr const char* FMT_SHM_SET_SIZE_FAILED = "Failed to set size for shared memory {}, errno: {} ({})";
constexpr const char* FMT_SHM_MMAP_FAILED = "Failed to map shared memory {}, errno: {} ({})";
constexpr const char* FMT_SHM_MUNMAP_FAILED = "Failed to unmap shared memory {}, errno: {} ({})";
constexpr const char* FMT_SHM_UNLINK_FAILED = "Failed to unlink shared memory {}, errno: {} ({})";


template <bool IsOwner>
class PosixSharedMemory {
public:
    PosixSharedMemory(const std::string& name, size_t size)
        : shmName_(nullptr), size_(size), ptr_(nullptr) {
        shmName_ = new char[name.size() + 1]; // +1 for '\0'
        std::strcpy(shmName_, name.c_str());

        constexpr int oflag = O_RDWR | (IsOwner ? O_CREAT : 0);
        int shm_fd = ::shm_open(shmName_, oflag, SHM_PERM_MODE);
        if (shm_fd == -1) {
            THROW_CERROR(FMT_SHM_CREATE_FAILED, shmName_);
        }
        
        if constexpr (IsOwner) {
            if (::ftruncate(shm_fd, size_) == -1) {
                ::shm_unlink(shmName_);
                THROW_CERROR(FMT_SHM_SET_SIZE_FAILED, shmName_);
            }
        }

        ptr_ = ::mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr_ == MAP_FAILED) {
            ::shm_unlink(shmName_);
            THROW_CERROR(FMT_SHM_MMAP_FAILED, shmName_);
        }

        ::close(shm_fd);
    }

    PosixSharedMemory(const PosixSharedMemory&) = delete;

    PosixSharedMemory& operator=(const PosixSharedMemory&) = delete;

    PosixSharedMemory(PosixSharedMemory&& other) {
        shmName_ = other.shmName_;
        other.shmName_ = nullptr;
        size_ = other.size_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    PosixSharedMemory& operator=(PosixSharedMemory&& other) {
        shmName_ = other.shmName_;
        other.shmName_ = nullptr;
        size_ = other.size_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    }

    ~PosixSharedMemory() {
        if (ptr_) {
            if (munmap(ptr_, size_) != 0) {
                LOG_CERROR(FMT_SHM_MUNMAP_FAILED, shmName_);
            }

        }
        if constexpr (IsOwner) {
            if (shm_unlink(shmName_) != 0) {
                LOG_CERROR(FMT_SHM_UNLINK_FAILED, shmName_);
            }
        }
        delete[] shmName_;
    }

    void* address() const {
        return ptr_;
    }

    size_t size() const {
        return size_;
    }

    static const mode_t SHM_PERM_MODE;

private:
    char *shmName_;
    size_t size_;
    void* ptr_;
};

template<bool IsOwner>
const mode_t PosixSharedMemory<IsOwner>::SHM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // 666

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
