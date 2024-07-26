#include "dumbtrader/ipc/posix_semaphore.h"

#include <iostream>
#include <stdexcept>

#include <semaphore.h>
#include <cerrno>       // errno macro (`int * __error(void)`)
#include <cstring>      // strerror function (errno to errmsg), strcpy
#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, ...)


namespace dumbtrader{

const mode_t PosixNamedSemaphore::SEM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 644

const unsigned int PosixNamedSemaphore::INITIAL_SEM_VALUE = 0;

PosixNamedSemaphore::PosixNamedSemaphore(const std::string &semName) {
    semName_ = new char[semName.size() + 1]; // +1 for '\0'
    std::strcpy(semName_, semName.c_str());
    sem_ = sem_open(semName_, O_CREAT, SEM_PERM_MODE, INITIAL_SEM_VALUE);
    if (sem_ == SEM_FAILED) {
        throw std::runtime_error("Failed to initialize semaphore, errno: " + std::to_string(errno) + " (" + std::strerror(errno) + ")");
    }
}

PosixNamedSemaphore::~PosixNamedSemaphore() {
    if (sem_unlink(semName_) != 0 && errno != ENOENT) { // ENOENT if already unlinked elsewhere
        std::cerr << "Failed to unlink semaphore \"" << semName_ << "\", errno: " << std::to_string(errno) << " (" << std::strerror(errno) << ")\n";
    }
    if (sem_close(sem_) != 0) {
        std::cerr << "Failed to close discriptor for semaphore \"" << semName_ << "\", errno: " << std::to_string(errno) << " (" << std::strerror(errno) << ")\n";
    }
    delete[] semName_;
}

// 等待信号量：如果信号量的值为0，调用此方法将阻塞线程直到信号量的值大于0
void PosixNamedSemaphore::wait() {
    if (sem_wait(sem_) != 0) {
        throw std::runtime_error("Failed to wait on semaphore");
    }
}

// 尝试等待信号量：如果信号量的值为0，调用此方法将返回 false 而不是阻塞线程
bool PosixNamedSemaphore::tryWait() {
    return sem_trywait(sem_) == 0;
}

// void PosixNamedSemaphore::timedWait(int milliSec) {
//     struct timespec ts;
//     if (sem_timedwait(sem_, &ts) != 0) {
//         throw std::runtime_error("Failed to wait on semaphore with timeout: " + std::to_string(milliSec) + "ms");
//     }
// }

void PosixNamedSemaphore::signal() {
    if (sem_post(sem_) != 0) {
        throw std::runtime_error("Failed to signal semaphore");
    }
}


} // namespace dumbtrader