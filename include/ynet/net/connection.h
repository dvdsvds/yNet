#pragma once

#include <cstdint>
#include <openssl/ssl.h>
#include <string>
#include <sys/types.h>
namespace ynet {
    class Connection {
        private:
            static constexpr int MAX_HANDSHAKE_RETRIES = 5;
            int client_fd;
            std::string client_ip;
            uint16_t client_port;
            SSL* ssl = nullptr;
        public:
            Connection(int client_fd, const std::string& client_ip, uint16_t client_port) : client_fd(client_fd), client_ip(client_ip), client_port(client_port) {}
            Connection(const Connection&) = delete;
            Connection& operator = (const Connection&) = delete;
            Connection(Connection&&) noexcept;
            Connection& operator = (Connection&&) noexcept;
            ~Connection();

            void setSSL(SSL* s) { ssl = s; }
            ssize_t read(char* buf, size_t len);
            ssize_t write(const char* buf, size_t len);
            void close();
            bool handshake();

            int getClientFd() const {
                return client_fd;
            }
            const std::string& getClientIP() const {
                return client_ip;
            }
            uint16_t getClientPort() const {
                return client_port;
            }
            SSL* getSSL() const { return ssl; }
    };
}
