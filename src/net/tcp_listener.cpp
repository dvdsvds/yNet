#include <memory>
#include <ynet/net/tcp_listener.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

using namespace ynet;

int TcpListener::bind() {
    server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) {
        return -1;
    }

    int opt = 1;
    int sockopt = ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(sockopt == -1) {
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config.port);
    if(::inet_pton(AF_INET, config.bind_addr.c_str(), &addr.sin_addr) != 1) {
        return -1;
    }

    return ::bind(server_fd, (sockaddr*)&addr, sizeof(addr));
}

int TcpListener::listen() {
    return ::listen(server_fd, config.max_connections);
}

std::unique_ptr<Connection> TcpListener::accept() {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(server_fd, (sockaddr*)&client_addr, &addr_len);
    if(client_fd == -1) {
        return nullptr;
    }
    char ip[INET_ADDRSTRLEN]; 
    ::inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    uint16_t port = ntohs(client_addr.sin_port);
    
    return std::make_unique<Connection>(client_fd, ip, port);
}

void TcpListener::close() {
    ::close(server_fd);
    server_fd = -1;
}
