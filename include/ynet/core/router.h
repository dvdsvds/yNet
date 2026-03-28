#pragma once

#include <unordered_map>
#include <functional>
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/cache/cache.h"

namespace ynet {
    using Handler = std::function<void(Request&, Response&)>;
    class Router;
    class Server;

    class Route {
        private:
            Router& router;
            std::string method;
            std::string path;
        public:
            Route(Router& r, const std::string& method, const std::string& path)
                : router(r), method(method), path(path) {}
            void html(const std::string& content);
            void json(const std::string& content);
            void file(const std::string& filepath);
            void handle(Handler fn);
    };

    class ErrorRoute {
        private:
            Server& server;
            int code;
        public:
            ErrorRoute(Server& s, int code) : server(s), code(code) {}
            void html(const std::string& content);
            void json(const std::string& content);
            void file(const std::string& filepath);
    };

    class Router {
        private:
            Cache& cache;
            std::unordered_map<std::string, Handler> routes;
        public:
            Router(Cache& cache) : cache(cache) {}
            Route get(const std::string& path);
            Route post(const std::string& path);
            Route put(const std::string& path);
            Route del(const std::string& path);

            void addRoute(const std::string& method, const std::string& path, Handler handler);
            
            std::optional<Handler> resolve(const std::string& method, const std::string& path) const;
            std::string loadFile(const std::string& filepath) { return cache.loadFile(filepath); }
    };

}
