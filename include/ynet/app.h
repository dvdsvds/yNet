#pragma once

#include "ynet/core/config.h"
#include "ynet/security/csrf.h"
#include "ynet/security/rate_limiter.h"
#include "ynet/security/sanitizer.h"
#include "ynet/security/secure_headers.h"
#include "ynet/util/config_parser.h"
#include "ynet/core/router.h"
#include "ynet/core/server.h"
#include "ynet/security/cors.h"
#include "ynet/util/logger.h"
#include "ynet/security/session.h"

namespace ynet {
    class App {
        private:
            Config config;
            Router router;
            Server server;
        public:
            App() : config(loadConfig()), server(config) {}
            Route get(const std::string& path) { return router.get(path); }
            Route post(const std::string& path) { return router.post(path); }
            Route put(const std::string& path) { return router.put(path); }
            Route del(const std::string& path) { return router.del(path); }

            void cors(const std::string& origin) { server.use(ynet::cors({origin})); }
            void rateLimit(int max_requests, int period_seconds) { server.use(ynet::RateLimiter(max_requests, period_seconds)); };
            void logger() { server.use(ynet::Logger()); }
            void csrf() { server.use(ynet::Csrf()); }
            void sanitizer() { server.use(ynet::Sanitizer()); }
            void secureHeaders() { server.use(ynet::SecureHeaders()); }
            void session() { server.use(ynet::Session()); }
            void serverStatic(const std::string& prefix, const std::string& dir) { server.serveStatic(prefix, dir); }
            void ws(const std::string& path, WsHandler handler) { server.ws(path, handler); }
            ErrorRoute onError(int code);
            void listen();
    };
}
