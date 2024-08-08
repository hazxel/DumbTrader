#include "dumbtrader/order_book/order_book.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <sched.h>



using dumbtrader::orderbook::OrderBook;
using dumbtrader::orderbook::OrderSide;

int main() {
    OrderBook book;
    
    std::string filename = "../book.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "cannot open file: " << filename << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 输出每一行内容
        std::istringstream iss(line);
        float px;
        float qty;
        int ord;
        iss >> px >> qty >> ord;
        std::cout << px << qty << ord << std::endl;
        book.update<OrderSide::ASK>(px, qty, ord);
    }



    // book.update<OrderSide::ASK>(2442.02, 90.4, 2);
    // book.update<OrderSide::ASK>(2441.99, 0, 0);
    // book.update<OrderSide::ASK>(2441.79, 45, 2);
    // book.update<OrderSide::ASK>(2441.78, 8.1, 1);
    // book.update<OrderSide::ASK>(2441.77, 0, 0);
    // book.update<OrderSide::ASK>(2441.73, 56, 2);
    // book.update<OrderSide::ASK>(2441.67, 32.9, 1);
    // book.update<OrderSide::ASK>(2441.64, 53.4, 5);
    // book.update<OrderSide::ASK>(2441.61, 0, 0);
    // book.update<OrderSide::ASK>(2441.53, 71.7, 4);
    // book.update<OrderSide::ASK>(2441.51, 112.6, 3);
    // book.update<OrderSide::ASK>(2441.48, 141.3, 4);
    return 0;
}