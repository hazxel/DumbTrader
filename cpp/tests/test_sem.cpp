#include <thread>
#include <iostream>

#include "dumbtrader/ipc/posix_semaphore.h"

void produce() {
    auto sem = dumbtrader::ipc::PosixNamedSemaphore<true>("/mysem");
    std::cout << "before signal\n";
    sem.signal();
    std::cout << "signal success, wait 1s and quit...\n"; // in case consumer sem not opened
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void consume() {
    auto sem = dumbtrader::ipc::PosixNamedSemaphore<false>("/mysem");
    std::cout << "before wait\n";
    sem.wait();
    std::cout << "wait success\n";
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