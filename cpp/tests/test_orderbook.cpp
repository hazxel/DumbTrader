#include "dumbtrader/order_book/order_book.h"

#include <fstream>
#include <iostream>
#include <sstream>

// #include <sched.h>

using dumbtrader::orderbook::OrderBook;
using dumbtrader::orderbook::OrderSide;

#ifdef __x86_64__
inline uint64_t rdtscp() {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi) :: "%rcx");
    return ((uint64_t)hi << 32) | lo;
}
#else
inline uint64_t rdtscp() {
    std::cout << "rdtscp not supported" << std::endl;
    return 0;
}
#endif

int main() {
    OrderBook book;
    
    std::string filename = "../book.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "cannot open file: " << filename << std::endl;
        return 1;
    }

    uint64_t start = rdtscp();

    std::string line;
    while (std::getline(file, line)) {
        // 输出每一行内容
        std::istringstream iss(line);
        float px;
        float qty;
        int ord;
        iss >> px >> qty >> ord;
        book.update<OrderSide::ASK>(px, qty, ord);
    }

    uint64_t end = rdtscp();
    std::cout << "CPU cycles: " << end - start << std::endl;
    
    return 0;
}