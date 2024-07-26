#ifndef DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
#define DUMBTRADER_IPC_POSIX_SEMAPHORE_H_

#include <string>
#include <iostream>
#include <stdexcept>

#include <cerrno>       // errno macro (`int * __error(void)`)
#include <cstring>      // strerror function (errno to errmsg), strcpy
#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, ...)
#include <semaphore.h>
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...), mode_t


namespace dumbtrader{

template <bool IsOwner>
class PosixNamedSemaphore {
public:
    PosixNamedSemaphore(const std::string &semName, int initial_value = DEFAULT_INIT_VALUE) {
        semName_ = new char[semName.size() + 1]; // +1 for '\0'
        std::strcpy(semName_, semName.c_str());
        sem_ = sem_open(semName_, O_CREAT, SEM_PERM_MODE, initial_value);
        if (sem_ == SEM_FAILED) {
            throw std::runtime_error("Failed to initialize semaphore, errno: " + std::to_string(errno) + " (" + std::strerror(errno) + ")");
        }
    }

    ~PosixNamedSemaphore() {
        if constexpr (IsOwner) {
            if (sem_unlink(semName_) != 0) {
                std::cerr << "Failed to unlink semaphore \"" << semName_ << "\", errno: " << std::to_string(errno) << " (" << std::strerror(errno) << ")\n";
            }
        }
        if (sem_close(sem_) != 0) {
            std::cerr << "Failed to close discriptor for semaphore \"" << semName_ << "\", errno: " << std::to_string(errno) << " (" << std::strerror(errno) << ")\n";
        }
        delete[] semName_;
    }

    // 等待信号量：如果信号量的值为0，调用此方法将阻塞线程直到信号量的值大于0
    void wait() {
        if (sem_wait(sem_) != 0) {
            throw std::runtime_error("Failed to wait on semaphore");
        }
    }

    // 尝试等待信号量：如果信号量的值为0，调用此方法将返回 false 而不是阻塞线程
    bool tryWait() {
        return sem_trywait(sem_) == 0;
    }

    // void timedWait(int milliSec) {
    //     struct timespec ts;
    //     if (sem_timedwait(sem_, &ts) != 0) {
    //         throw std::runtime_error("Failed to wait on semaphore with timeout: " + std::to_string(milliSec) + "ms");
    //     }
    // }

    void signal() {
        if (sem_post(sem_) != 0) {
            throw std::runtime_error("Failed to signal semaphore");
        }
    }

    static const unsigned int DEFAULT_INIT_VALUE;

    static const mode_t SEM_PERM_MODE;

private:
    sem_t *sem_;
    char *semName_;
};

template<bool IsOwner>
const mode_t PosixNamedSemaphore<IsOwner>::SEM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 644

template<bool IsOwner>
const unsigned int PosixNamedSemaphore<IsOwner>::DEFAULT_INIT_VALUE = 0;

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
