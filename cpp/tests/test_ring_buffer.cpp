#include "dumbtrader/ipc/posix_shared_memory_ring_buffer.h"

#include<iostream>
#include<thread>

void produce() {
    auto ringBuffer = dumbtrader::ipc::PosixSharedMemoryRingBuffer<int, true>("/my_int_buffer", 10);
    for(int i = 0; i < 20; ++i) {
        ringBuffer.put(i);
    }
}

void consume() {
    auto ringBuffer = dumbtrader::ipc::PosixSharedMemoryRingBuffer<int, false>("/my_int_buffer", 10);
    for(int i = 0; i < 20; ++i) {
       std::cout << ringBuffer.get() << ',';
    }
}

int main() {
    auto t1 = std::thread(produce);
    auto t2 = std::thread(consume);
    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
    return 0;
}