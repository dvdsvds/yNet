#pragma once

#include <memory>
#include <vector>
#include <ynet/middleware.h>
#include "ynet/config.h"
#include "ynet/router.h"
#include "ynet/net/tcp_listener.h"
#include "ynet/security/tls_context.h"
#include "ynet/net/thread_pool.h"
#include "ynet/net/event_loop.h"
#include "ynet/net/websocket.h"

namespace ynet {
    using WsHandler = std::function<void(WebSocket&)>;
    class Server {
        private:
            Config config;
            Router router;
            TcpListener tcp_listener;
            std::vector<Middleware> middlewares;
            std::unique_ptr<TlsContext> tls_ctx;
            EventLoop ev;
            std::unordered_map<int, std::unique_ptr<Connection>> connections;
            ThreadPool tp;
            std::mutex conn_mtx;
            std::unordered_map<std::string, WsHandler> ws_routes;
        public:
            Server(const Config& config) :
                config(config), tcp_listener(config), ev(config.max_connections), tp(std::thread::hardware_concurrency()) {
                    if(config.use_tls) {
                        tls_ctx = std::make_unique<TlsContext>(config.cert_path, config.key_path);
                    }
                }
            void start();
            void stop();
            void mount(Router& r);
            void use(Middleware mw);
            void ws(const std::string& path, WsHandler handler);
    };
}
