#pragma once

#include <memory>
#include <vector>
#include "ynet/core/middleware.h"
#include "ynet/core/config.h"
#include "ynet/core/router.h"
#include "ynet/core/static_file.h"
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
            Router* router = nullptr;
            TcpListener tcp_listener;
            std::vector<Middleware> middlewares;
            std::unique_ptr<TlsContext> tls_ctx;
            EventLoop ev;
            std::unordered_map<int, std::shared_ptr<Connection>> connections;
            ThreadPool tp;
            std::mutex conn_mtx;
            std::unordered_map<std::string, WsHandler> ws_routes;
            std::vector<StaticFileServer> static_files;
            std::unordered_map<int, std::function<Response(const Request&)>> error_handlers;
            void handleRequest(int fd, Connection& conn);
            void handleWebSocket(Connection& conn, Request& req);
            std::atomic<bool> running{true};
        public:
            Server(const Config& config) :
                config(config), tcp_listener(config), ev(config.max_connections), tp(std::thread::hardware_concurrency()) {
                    if(config.use_tls) {
                        tls_ctx = std::make_unique<TlsContext>(config.cert_path, config.key_path);
                    }
                }
            void start();
            void stop();
            void mount(Router& r) { router = &r; }
            void use(Middleware mw) { middlewares.push_back(mw); }
            void ws(const std::string& path, WsHandler handler) { ws_routes[path] = handler; }
            void serveStatic(const std::string& prefix, const std::string& dir) { static_files.emplace_back(prefix, dir); }
            void onError(int code, std::function<Response(const Request&)> handler) { error_handlers[code] = handler; }
    };
}
