#pragma once
#include <memory>
#include <ynet/config.h>
#include <ynet/net/connection.h>

namespace ynet {
    class TcpListener {
        private:
            int server_fd;
            Config config;            
        public:
            TcpListener(const Config& config) :
                config(config) {}
            int bind();
            int listen();
            std::unique_ptr<Connection> accept();
            void close();
            int getFd() const { return server_fd; }
    };
}
