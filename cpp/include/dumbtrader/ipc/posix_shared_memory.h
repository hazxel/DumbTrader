#ifndef DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
#define DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_

#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, O_RDWR, ...)
#include <stdexcept>
#include <string>
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...)
#include <iostream>


#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace dumbtrader{

class PosixSharedMemory {
public:
    PosixSharedMemory(const std::string& name, size_t size, bool create = false);

    ~PosixSharedMemory();

    void* get() const;

    size_t size() const;

private:
    std::string name_;
    size_t size_;
    int shm_fd_;
    void* ptr_;
};

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SHARED_MEMORY_H_
