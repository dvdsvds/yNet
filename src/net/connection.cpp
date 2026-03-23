#include <ynet/net/connection.h>
#include <unistd.h>
#include <sys/socket.h>

using namespace ynet;

ssize_t Connection::read(char* buf, size_t len) {
    if(ssl) return SSL_read(ssl, buf, len);
    return ::recv(client_fd, buf, len, 0);
}

ssize_t Connection::write(const char* buf, size_t len) {
    if(ssl) return SSL_write(ssl, buf, len);
    return ::send(client_fd, buf, len, 0);
}

void Connection::close() {
    if(ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }
    ::close(client_fd);
    client_fd = -1;
}

Connection::~Connection() {
    if(client_fd != -1) {
        close();
    }
}

