#include "dumbtrader/ipc/posix_shared_memory.h"
#include "dumbtrader/utils/error.h"

#include <cstring>      // std::strcpy
#include <iostream>

#include <sys/wait.h>   // wait
#include <unistd.h>     // fork


void create() { // sleep 10s to check /dev/shm
    auto shm = new dumbtrader::ipc::PosixSharedMemory<true>("/my_shm", 15);
    ::sleep(10);
    delete shm;
}

void produce() {
    // prevent compiler release shm before sleep
    auto shm = new dumbtrader::ipc::PosixSharedMemory<true>("/my_shm", 15);
    char* c = static_cast<char*>(shm->address());
    std::strcpy(c, "Hello, World!");
    std::cout << "producer written to shared memory, wait 2s...\n"; // in case consumer shm not opened
    ::sleep(2);
    std::cout << "producer quit...\n";
    delete shm;
}

void consume() {
    std::cout << "consumer wait 1s...\n"; // in case producer shm not opened
    ::sleep(1);
    auto shm = dumbtrader::ipc::PosixSharedMemory<false>("/my_shm", 15);
    char* c = static_cast<char*>(shm.address());
    std::cout << "consumer read shared memory: " << c << std::endl;
}

int main() {
    // create();
    pid_t pid = ::fork();
    if (pid < 0) {
        THROW_RUNTIME_ERROR("Fork failed");
        return 1;
    } else if (pid == 0) {
        produce();
        ::_exit(0);
    } else {
        consume();
        ::wait(nullptr);
    }
    return 0;
}