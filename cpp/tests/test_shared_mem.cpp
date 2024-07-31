#include "dumbtrader/ipc/posix_shared_memory.h"
#include "dumbtrader/utils/error.h"

#include<iostream>
#include<thread>

#include<unistd.h>

void create() { // sleep 10s to check /dev/shm
    auto shm = new dumbtrader::ipc::PosixSharedMemory<true>("/my_shm", 15);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    delete shm;
}

void produce() {
    // prevent compiler release shm before sleep
    auto shm = new dumbtrader::ipc::PosixSharedMemory<true>("/my_shm", 15);
    char* c = static_cast<char*>(shm->address());
    std::strcpy(c, "Hello, World!");
    std::cout << "producer written to shared memory, wait 2s...\n"; // in case consumer shm not opened
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "producer quit...\n";
    delete shm;
}

void consume() {
    std::cout << "consumer wait 1s...\n"; // in case producer shm not opened
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto shm = dumbtrader::ipc::PosixSharedMemory<false>("/my_shm", 15);
    char* c = static_cast<char*>(shm.address());
    std::cout << "consumer read shared memory: " << c << std::endl;
}

int main() {
    // create();
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