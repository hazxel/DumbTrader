#include "dumbtrader/network/socket.h"
#include "dumbtrader/utils/error.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include <iostream>

namespace dumbtrader::network {

void construct_sockaddr_in(struct sockaddr_in& addr, const char* ip, int port) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}

Socket::Socket() : sockfd_(-1) {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_CREATE_FAILED);
    }
}

Socket::~Socket() {
    if (sockfd_ != -1) {
        close(sockfd_);
        sockfd_ = -1;
    }
}

void Socket::bind(const char* ip, int port) {
    construct_sockaddr_in(addr_, ip, port);
    if(::bind(sockfd_, (sockaddr*) &addr_, sizeof(addr_)) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_BIND_FAILED, ip, port);
    }
}

void Socket::listen(int backlog) {
    if(::listen(sockfd_, backlog) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_LISTEN_FAILED);
    }
}

void Socket::connect(const char* ip, int port) {
    construct_sockaddr_in(addr_, ip, port);
    if(::connect(sockfd_, (sockaddr*) &addr_, sizeof(addr_)) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_CONNECT_FAILED, ip, port);
    }
}

// int Socket::accept() {
//     struct sockaddr_in clnt_addr;
//     memset(&clnt_addr, 0, sizeof(clnt_addr));
//     socklen_t clnt_addr_len = sizeof(clnt_addr);
//     int clnt_sockfd = ::accept(sockfd_, (sockaddr*)&clnt_addr, &clnt_addr_len);
//     errif(clnt_sockfd == -1, MSG_SOCKET_ACCEPT_ERR);
//     std::cout << "accepted a new connection! client fd: " << clnt_sockfd << ", IP: " << inet_ntoa(clnt_addr.sin_addr) << ", Port: " << ntohs(clnt_addr.sin_port) << std::endl;
//     return clnt_sockfd;
// }

// // set socket to non-blocking mode, only after connection is established
// void Socket::set_nonblock() {
//     int flags = fcntl(sockfd_, F_GETFL, 0);
//     errif(flags == -1, "fcntl get flags error");
//     flags |= O_NONBLOCK;
//     int ret = fcntl(sockfd_, F_SETFL, flags);
//     errif(ret == -1, "fcntl set flags error");
// }

} // namespace dumbtrader::network