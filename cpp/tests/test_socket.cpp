#include "dumbtrader/network/socket.h"
#include "dumbtrader/utils/error.h"

#include <cstring>
#include <iostream>

#include <unistd.h>     // fork

using namespace dumbtrader::network;

constexpr const char DEFAULT_SERVER_IP[] = "127.0.0.1";
constexpr const int  DEFAULT_SERVER_PORT = 8888;
constexpr const int  BUFFER_SIZE = 16;

void run_client() {
    Socket<Side::CLIENT> client_socket;
    client_socket.connect(DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);
    int sockfd = client_socket.get_fd();

    char buf[BUFFER_SIZE];
    std::memset(buf, 0, sizeof(buf));
    std::strcpy(buf, "Hello!");
    if(write(sockfd, buf, sizeof(buf)) == -1) {
        THROW_RUNTIME_ERROR("Failed to write socket, connection may be closed. errno: {} ({})");
    }  

    memset(buf, 0, sizeof(buf));
    ssize_t read_bytes = read(sockfd, buf, sizeof(buf));

    if (read_bytes > 0){
        std::cout << "Client received message from server: " << buf << std::endl;
    } else if (read_bytes == 0) {
        std::cout << "server socket disconnected" << std::endl;
    } else if (read_bytes == -1) {
        THROW_RUNTIME_ERROR("Failed to read socket, errno: {} ({})");
    }
    
}

void run_server() {
    Socket<Side::SERVER> server_socket;
    server_socket.bind(DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);

    server_socket.listen(1);

    int client_socket = server_socket.accept();

    char buf[BUFFER_SIZE];
    std::memset(buf, 0, sizeof(buf));
    ssize_t read_bytes = read(client_socket, buf, sizeof(buf));

    std::cout << "Server received message from client: " << buf << std::endl;
    send(client_socket, buf, read_bytes, 0);
    
    close(client_socket);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        THROW_RUNTIME_ERROR("Fork failed");
        return 1;
    } else if (pid == 0) {
        run_server();
        _exit(0);
    } else {
        sleep(1);
        run_client();
        wait(nullptr);
    }
    return 0;
}