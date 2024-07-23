#ifndef DUMBTRADER_IPC_POSIX_WRAPPER_H_
#define DUMBTRADER_IPC_POSIX_WRAPPER_H_

#include <semaphore.h>

#include <stdexcept>
#include <cerrno>    // 包含 errno
#include <cstring>   // 包含 strerror
#include <string>

class POSIXNamedSemaphore {
public:
    POSIXNamedSemaphore(const std::string semName) {
        semName_ = new char[semName.size() + 1]; // +1 for '\0'
        std::strcpy(semName_, semName.c_str());
        sem_ = sem_open(semName_, O_CREAT, 0644, 1);
        if (sem_ == SEM_FAILED) {
            throw std::runtime_error("Failed to initialize semaphore, errno: " + std::to_string(errno) + " (" + std::strerror(errno) + ")");
        }
    }

    ~POSIXNamedSemaphore() {
        if (sem_destroy(sem_) != 0) {
            throw std::runtime_error("Failed to destroy semaphore, errno: " + std::to_string(errno) + " (" + std::strerror(errno) + ")");
        }
        if (sem_unlink(semName_) != 0) {
            throw std::runtime_error("Failed to unlink semaphore, errno: " + std::to_string(errno) + " (" + std::strerror(errno) + ")");
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

    void signal() {
        if (sem_post(sem_) != 0) {
            throw std::runtime_error("Failed to signal semaphore");
        }
    }

private:
    sem_t *sem_;
    char *semName_;
};


#endif // DUMBTRADER_IPC_POSIX_WRAPPER_H_
