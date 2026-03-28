#pragma once

#include <filesystem>
#include <functional>
#include <unordered_map>
#include <ynet/request.h>
#include <ynet/response.h>

namespace fs = std::filesystem;

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

    struct FileCache {
        std::string content;
        fs::file_time_type mtime;
    };

    class Router {
        private:
            std::unordered_map<std::string, Handler> routes;
            std::unordered_map<std::string, FileCache> file_cache;
        public:
            Route get(const std::string& path);
            Route post(const std::string& path);
            Route put(const std::string& path);
            Route del(const std::string& path);

            void addRoute(const std::string& method, const std::string& path, Handler handler);
            
            std::optional<Handler> resolve(const std::string& method, const std::string& path) const;
            std::string loadFile(const std::string& filepath);
    };

}
