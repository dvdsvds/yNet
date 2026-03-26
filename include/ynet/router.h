#pragma once

#include <functional>
#include <unordered_map>
#include <ynet/request.h>
#include <ynet/response.h>

namespace ynet {
    using Handler = std::function<void(Request&, Response&)>;
    class Router {
        private:
            std::unordered_map<std::string, Handler> routes;
        public:
            void get(const std::string& path, Handler handler);
            void post(const std::string& path, Handler handler);
            void put(const std::string& path, Handler handler);
            void del(const std::string& path, Handler handler);
            std::optional<Handler> resolve(const std::string& method, const std::string& path) const;
    };
}
