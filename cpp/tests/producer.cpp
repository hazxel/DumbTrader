#include "dumbtrader/ipc/posix_wrapper.h"

int main() {
    auto sem = dumbtrader::POSIXNamedSemaphore("/mysem");
    sem.signal();
    return 0;
}