#ifndef DUMBTRADER_NETWORK_SOCKET_H_
#define DUMBTRADER_NETWORK_SOCKET_H_

#include "dumbtrader/utils/error.h"

#include <cstring>      // std::memset

#include <arpa/inet.h>  // inet_addr (convert `char*` ip addr to `__uint32_t`)
#include <fcntl.h>      // fnctl
#include <netinet/in.h> // sockaddr_in (specific descriptor for IPv4 communication)
#include <sys/socket.h> // sockaddr (generic descriptor for any socket operation), connect, listen, bind, accept
#include <unistd.h>     // close


namespace dumbtrader::network {
    
constexpr const char* FMT_SOCKET_CREATE_FAILED = "Failed to create socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_SET_REUSEADDR_FAILED = "Failed to set SO_REUSEADDR for socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_BIND_FAILED = "Failed to bind socket to {}:{}, errno: {} ({})";
constexpr const char* FMT_SOCKET_LISTEN_FAILED = "Failed to listen to socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_CONNECT_FAILED = "Failed to connect to socket server {}:{}, errno: {} ({})";
constexpr const char* FMT_SOCKET_ACCEPT_FAILED = "Failed to accept socket connection, errno: {} ({})";
constexpr const char* FMT_SOCKET_SEND_FAILED = "Failed to send message to socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_RECV_FAILED = "Failed to receive message from socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_RECV_PEER_DISCONNECTED = "Failed to receive message from socket, peer disconnected, errno: {} ({})";

namespace detail {
inline void construct_sockaddr_in(struct sockaddr_in& addr, const char* ip, int port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}
}

enum class Side {
    SERVER,
    CLIENT
};

template<Side = Side::CLIENT, bool IsBlock = false>
class Socket {
public:
    Socket() : sockfd_(-1) {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ == -1) {
            THROW_RUNTIME_ERROR(FMT_SOCKET_CREATE_FAILED);
        }
    }

    Socket(int sockfd) : sockfd_(sockfd) {
        if (sockfd_ == -1) {
            THROW_RUNTIME_ERROR(FMT_SOCKET_CREATE_FAILED);
        }
    } 

    ~Socket() {
        if (sockfd_ != -1) {
            close(sockfd_);
            sockfd_ = -1;
        }
    }

    // server side only functions
    template<bool EnableAddrReuse = true>
    void bind(const char* ip, int port);

    void listen(int backlog);

    int accept();

    void set_nonblock();
    // end of server side only functions

    // client side only functions
    void connect(const char* ip, int port);
    // end of client side only functions

    ssize_t send(const void* buffer, size_t length, int flags = 0) const {
        ssize_t bytesSent = ::send(sockfd_, buffer, length, flags);
        if (bytesSent < 0) {
            THROW_RUNTIME_ERROR(FMT_SOCKET_SEND_FAILED);
        }
        return bytesSent;
    }

    ssize_t recv(void* buffer, size_t length, int flags = 0) const {
        ssize_t bytesReceived = ::recv(sockfd_, buffer, length, flags);
        if (bytesReceived < 0) {
            THROW_RUNTIME_ERROR(FMT_SOCKET_RECV_FAILED);
        } else if (bytesReceived == 0) {
            THROW_RUNTIME_ERROR(FMT_SOCKET_RECV_PEER_DISCONNECTED);
        }
        return bytesReceived;
    }

    int get_fd() const { return sockfd_; }
private:
    int sockfd_;
    struct sockaddr_in addr_;
};

template<>
void Socket<Side::CLIENT>::connect(const char* ip, int port) {
    detail::construct_sockaddr_in(addr_, ip, port);
    if(::connect(sockfd_, (sockaddr*) &addr_, sizeof(addr_)) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_CONNECT_FAILED, ip, port);
    }
}

template<>
template<bool EnableAddrReuse>
void Socket<Side::SERVER>::bind(const char* ip, int port) {
    if constexpr (EnableAddrReuse) {
        int opt = 1;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(sockfd_);
            THROW_RUNTIME_ERROR(FMT_SOCKET_SET_REUSEADDR_FAILED);
        }
    }

    detail::construct_sockaddr_in(addr_, ip, port);
    if(::bind(sockfd_, (sockaddr*) &addr_, sizeof(addr_)) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_BIND_FAILED, ip, port);
    }
}

template<>
void Socket<Side::SERVER>::listen(int backlog) {
    if(::listen(sockfd_, backlog) == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_LISTEN_FAILED);
    }
}

template<>
int Socket<Side::SERVER>::accept() {
    struct sockaddr_in clnt_addr;
    std::memset(&clnt_addr, 0, sizeof(clnt_addr));
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int clnt_sockfd = ::accept(sockfd_, (sockaddr*)&clnt_addr, &clnt_addr_len);
    if(clnt_sockfd == -1) {
        THROW_RUNTIME_ERROR(FMT_SOCKET_ACCEPT_FAILED);
    }
    return clnt_sockfd;
}

// set socket to non-blocking mode, only after connection is established
template<>
void Socket<Side::SERVER>::set_nonblock() {
    int flags = fcntl(sockfd_, F_GETFL, 0);
    if(flags == -1) {
        THROW_RUNTIME_ERROR("Failed to get fcntl flags");
    }
    flags |= O_NONBLOCK;
    if(fcntl(sockfd_, F_SETFL, flags) == -1) {
        THROW_RUNTIME_ERROR("Failed to set fcntl flags");
    }
}

} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_SOCKET_H_