#ifndef DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
#define DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_

#include <string>
#include <sys/mman.h>   // shared memory
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...), mode_t


namespace dumbtrader{

class PosixSharedMemory {
public:
    PosixSharedMemory(const std::string& name, size_t size, bool create = false);

    ~PosixSharedMemory();

    void* get() const;

    size_t size() const;

    static const mode_t SHM_PERM_MODE;

private:
    char *shmName_;
    size_t size_;
    int shm_fd_;
    void* ptr_;
};

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
