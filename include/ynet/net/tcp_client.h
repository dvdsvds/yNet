#pragma once

#include <cstdint>
#include <string>
namespace ynet {
    class TcpClient {
        private:
            int fd;
        public:
            int connect(const std::string& ip, uint16_t port);
            int send(const std::string& data);
            int recv(char* buf, size_t len);
            void close();
    };
}
