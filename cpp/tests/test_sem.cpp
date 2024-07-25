#include <thread>
#include <iostream>

#include "dumbtrader/ipc/posix_wrapper.h"


void consume() {
    auto sem = dumbtrader::POSIXNamedSemaphore("/mysem");
    sem.wait();
    std::cout << "wait success" << std::endl;
}

void produce() {
    auto sem = dumbtrader::POSIXNamedSemaphore("/mysem");
    sem.signal();
    std::cout << "signal success" << std::endl;
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