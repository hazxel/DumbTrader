#include "dumbtrader/ipc/posix_shared_memory_ring_buffer.h"
#include "dumbtrader/utils/error.h"

#include <iostream>

#include <sys/wait.h>   // wait
#include <unistd.h>     // fork

void produce() {
    auto ringBuffer = dumbtrader::ipc::PosixSharedMemoryRingBuffer<int, true>("/my_int_buffer", 10);
    for(int i = 0; i < 20; ++i) {
        ringBuffer.put(i);
    }
}

void consume() {
    std::cout << "consumer wait 1s...\n"; // in case producer shm not opened
    ::sleep(1);
    auto ringBuffer = dumbtrader::ipc::PosixSharedMemoryRingBuffer<int, false>("/my_int_buffer", 10);
    for(int i = 0; i < 20; ++i) {
       std::cout << ringBuffer.get() << ',';
    }
}

int main() {
    pid_t pid = ::fork();
    if (pid < 0) {
        THROW_CERROR("Fork failed");
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