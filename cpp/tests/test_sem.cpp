#include "dumbtrader/ipc/posix_semaphore.h"
#include "dumbtrader/utils/error.h"

#include <iostream>

#include <sys/wait.h>   // wait
#include <unistd.h>     // fork


void produce() {
    // prevent compiler release sem before sleep
    auto sem = new dumbtrader::ipc::PosixNamedSemaphore<true>("/mysem");
    sem->signal();
    std::cout << "signaled, wait 1s and quit...\n"; // in case consumer sem not opened
    sleep(1);
    delete sem;
}

void consume() {
    auto sem = dumbtrader::ipc::PosixNamedSemaphore<false>("/mysem");
    std::cout << "waiting signal...\n";
    sem.wait();
    std::cout << "wait success\n";
}

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        THROW_RUNTIME_ERROR("Fork failed");
        return 1;
    } else if (pid == 0) {
        produce();
        _exit(0);
    } else {
        consume();
        wait(nullptr);
    }
    return 0;
}