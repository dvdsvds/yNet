#include <poll.h>
#include <openssl/err.h>
#include <ynet/net/connection.h>
#include <unistd.h>
#include <sys/socket.h>

using namespace ynet;

Connection::Connection(Connection&& other) noexcept 
    : client_fd(other.client_fd), client_ip(std::move(other.client_ip)), client_port(other.client_port), ssl(other.ssl) 
{
    other.client_fd = -1;
    other.client_port = 0;
    other.ssl = nullptr;
        
}

Connection& Connection::operator=(Connection&& other) noexcept {
    if(this != &other) {
        if(client_fd != -1) {
            ::close(client_fd);
        }
        if(ssl != nullptr) {
            SSL_free(ssl);
        }

        client_fd = other.client_fd;
        client_ip = std::move(other.client_ip);
        client_port = other.client_port;
        ssl = other.ssl;

        other.client_fd = -1;
        other.ssl = nullptr;
        other.client_port = 0;
    }
    return *this;
}

ssize_t Connection::read(char* buf, size_t len) {
    if(client_fd == -1) return -1;
    if(ssl) return SSL_read(ssl, buf, len);
    return ::recv(client_fd, buf, len, 0);
}

ssize_t Connection::write(const char* buf, size_t len) {
    if(client_fd == -1) return -1;
    if(ssl) return SSL_write(ssl, buf, len);
    return ::send(client_fd, buf, len, 0);
}

void Connection::close() {
    if(client_fd == -1) return; 

    if(ssl) {
        int ret = SSL_shutdown(ssl);
        if(ret == 0) {
            struct pollfd pfd;
            pfd.fd = client_fd;
            pfd.events = POLLIN;

            int poll_ret = poll(&pfd, 1, 1000);
            if(poll_ret > 0 && (pfd.revents & POLLIN)) {
                SSL_shutdown(ssl);
            }
        }
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
    for(int i = 0; i < MAX_HANDSHAKE_RETRIES; ++i) {
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
    return false;
}
