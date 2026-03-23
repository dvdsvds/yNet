#pragma once

#include <memory>
#include <vector>
#include <ynet/middleware.h>
#include <ynet/config.h>
#include <ynet/router.h>
#include <ynet/net/tcp_listener.h>
#include <ynet/security/tls_context.h>

namespace ynet {
    class Server {
        private:
            Config config;
            Router router;
            TcpListener tcp_listener;
            std::vector<Middleware> middlewares;
            std::unique_ptr<TlsContext> tls_ctx;
        public:
            Server(const Config& config) :
                config(config), tcp_listener(config) {
                    if(config.use_tls) {
                        tls_ctx = std::make_unique<TlsContext>(config.cert_path, config.key_path);
                    }
                }
            void start();
            void stop();
            void mount(Router& r);
            void use(Middleware mw);
    };
}
