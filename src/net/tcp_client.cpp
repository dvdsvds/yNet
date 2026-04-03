#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ynet/net/tcp_client.h>

using namespace ynet;

int TcpClient::connect(const std::string& ip, uint16_t port) {
    this->fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(this->fd == -1) {
        return -1;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        ::close(this->fd);
        this->fd = -1;
        return -1;
    }

    if(::connect(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        ::close(this->fd);
        this->fd = -1;
        return -1;
    }

    return 0;
}

int TcpClient::send(const std::string& data) {
    return ::send(this->fd, data.c_str(), data.size(), 0);
}

int TcpClient::recv(char* buf, size_t len) {
    return ::recv(this->fd, buf, len, 0);
} 

void TcpClient::close() {
    if(this->fd != 1) {
        ::close(this->fd);
        this->fd = -1;
    }
}
