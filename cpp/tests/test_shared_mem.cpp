#include "dumbtrader/ipc/posix_shared_memory.h"

#include<iostream>
#include<thread>

void produce() {
    auto shm = dumbtrader::ipc::PosixSharedMemory<true>("/my_shm", 15);
    char* c = static_cast<char*>(shm.get());
    std::strcpy(c, "Hello, World!");
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void consume() {
    auto shm = dumbtrader::ipc::PosixSharedMemory<false>("/my_shm", 15);
    char* c = static_cast<char*>(shm.get());
    std::cout << c << std::endl;
}

int main() {
    auto t1 = std::thread(produce);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto t2 = std::thread(consume);
    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
    return 0;
}