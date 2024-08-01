#ifndef DUMBTRADER_NETWORK_SOCKET_H_
#define DUMBTRADER_NETWORK_SOCKET_H_

#include <arpa/inet.h>

namespace dumbtrader::network {
    
// constexpr const char* FMT_SEM_CREATE_FAILED = "Failed to create semaphore {}, errno: {} ({})";

constexpr const char* FMT_SOCKET_CREATE_FAILED = "Failed to create socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_BIND_FAILED = "Failed to bind socket to {}:{}, errno: {} ({})";
constexpr const char* FMT_SOCKET_LISTEN_FAILED = "Failed to listen to socket, errno: {} ({})";
constexpr const char* FMT_SOCKET_CONNECT_FAILED = "Failed to connect to socket, errno: {} ({})";
// constexpr const char* FMT_SOCKET_ACCEPT_ERR = "socket accept error.";
// constexpr const char* FMT_SOCKET_READ_ERR = "socket read error.";
// constexpr const char* FMT_SOCKET_WRITE_ERR = "socket write error, connection may be closed.";
// constexpr const char* FMT_SERVER_SOCKET_DISCONNECTED = "server socket disconnected.";


void construct_sockaddr_in(struct sockaddr_in& addr, const char* ip, int port);

class Socket {
public:
    Socket();
    ~Socket();

    void bind(const char* ip, int port);
    void listen(int backlog);
    void connect(const char* ip, int port);
    // int accept();

    // void set_nonblock();

    inline int get_fd() const { return sockfd_; }
private:
    int sockfd_;
    struct sockaddr_in addr_;
};



} // namespace dumbtrader::network

#endif // DUMBTRADER_NETWORK_SOCKET_H_