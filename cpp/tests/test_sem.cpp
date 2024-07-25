#include <thread>
#include <iostream>

#include "dumbtrader/ipc/posix_semaphore.h"


void consume() {
    auto sem = dumbtrader::PosixNamedSemaphore("/mysem");
    std::cout << "before wait\n";
    sem.wait();
    std::cout << "wait success\n";
}

void produce() {
    auto sem = dumbtrader::PosixNamedSemaphore("/mysem");
    std::cout << "before signal\n";
    sem.signal();
    std::cout << "signal success\n";
}

int main() {
    auto t1 = std::thread(consume);
    auto t2 = std::thread(produce);
    if (t1.joinable()) {
        t1.join();
    }
    if (t2.joinable()) {
        t2.join();
    }
    return 0;
}