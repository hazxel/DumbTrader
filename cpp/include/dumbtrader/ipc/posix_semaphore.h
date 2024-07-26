#ifndef DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
#define DUMBTRADER_IPC_POSIX_SEMAPHORE_H_

#include <string>
#include <semaphore.h>
#include <sys/stat.h>   // symbolic definitions for the permissions bits (S_IRUSR, S_IWUSR, ...), mode_t


namespace dumbtrader{
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

    static const unsigned int INITIAL_SEM_VALUE;

    static const mode_t SEM_PERM_MODE;

private:
    sem_t *sem_;
    char *semName_;
};

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SEMAPHORE_H_
