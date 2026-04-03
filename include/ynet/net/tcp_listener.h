#pragma once

#include "ynet/core/config.h"
#include "ynet/net/connection.h"
#include <memory>

namespace ynet {
    class TcpListener {
        private:
            int server_fd = -1;
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
