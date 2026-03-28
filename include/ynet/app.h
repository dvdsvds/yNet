#pragma once

#include "ynet/cache/cache.h"
#include "ynet/core/config.h"
#include "ynet/util/config_parser.h"
#include "ynet/core/router.h"
#include "ynet/core/server.h"

namespace ynet {
    class App {
        private:
            Config config;
            Cache cache;
            Router router;
            Server server;
        public:
            App() : config(loadConfig()), cache(), router(cache), server(config) {}
            Route get(const std::string& path) { return router.get(path); }
            Route post(const std::string& path) { return router.post(path); }
            Route put(const std::string& path) { return router.put(path); }
            Route del(const std::string& path) { return router.del(path); }

            void cors(const std::string& origin);
            void rateLimit(int max_requests, int period_seconds);
            void logger();
            void csrf();
            void sanitizer();
            void secureHeaders();
            void session();
            void serveStatic(const std::string& prefix, const std::string& dir);
            void ws(const std::string& path, WsHandler handler);
            ErrorRoute onError(int code);
            void listen();
    };
}
