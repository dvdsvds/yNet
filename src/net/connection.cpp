#include <poll.h>
#include <openssl/err.h>
#include <ynet/net/connection.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

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

bool Connection::handshake() {
    if(!ssl) return false;
    while(true) {
        int ret = SSL_accept(ssl);
        if(ret == 1) return true;
        int err = SSL_get_error(ssl, ret);
        if(err == SSL_ERROR_WANT_READ) {
            struct pollfd pfd = {client_fd, POLLIN, 0};
            int pr = poll(&pfd, 1, 5000);
            if(pr <= 0) return false;
        } else if(err == SSL_ERROR_WANT_WRITE) {
            struct pollfd pfd = {client_fd, POLLOUT, 0};
            int pr = poll(&pfd, 1, 5000);
            if(pr <= 0) return false;
        } else {
            ERR_print_errors_fp(stderr);
            return false;
        }
    }
}
