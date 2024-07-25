#include "dumbtrader/ipc/posix_wrapper.h"

int main() {
    auto sem = dumbtrader::POSIXNamedSemaphore("/mysem");
    sem.wait();
    return 0;
}