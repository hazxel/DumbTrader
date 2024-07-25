#ifndef DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
#define DUMBTRADER_IPC_POSIX_SEMAPHORE_H_

#include <semaphore.h>

#include <cerrno>       // errno macro (`int * __error(void)`)
#include <cstring>      // strerror function (errno to errmsg)
#include <fcntl.h>      // file operation flags (O_CREAT, O_EXCL, ...)
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...)

#include <iostream>
#include <stdexcept>
#include <string>


namespace dumbtrader{

const unsigned int INITIAL_SEM_VALUE = 0;
const mode_t SEM_PERM_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

class PosixNamedSemaphore {
public:
    PosixNamedSemaphore(const std::string &semName);

    ~PosixNamedSemaphore();

    // 等待信号量：如果信号量的值为0，调用此方法将阻塞线程直到信号量的值大于0
    void wait();

    // 尝试等待信号量：如果信号量的值为0，调用此方法将返回 false 而不是阻塞线程
    bool tryWait();

    // void timedWait(int milliSec);

    void signal();

private:
    sem_t *sem_;
    char *semName_;
};

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
