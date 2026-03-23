#include <ynet/router.h>

using namespace ynet;

void Router::get(const std::string& path, Handler handler) {
    routes["GET:" + path] = handler;
}

void Router::post(const std::string& path, Handler handler) {
    routes["POST:" + path] = handler;
}

void Router::put(const std::string& path, Handler handler) {
    routes["PUT:" + path] = handler;
}

void Router::del(const std::string& path, Handler handler) {
    routes["DELETE:" + path] = handler;
}

std::optional<Handler> Router::resolve(const std::string& method, const std::string& path) const {
    auto it = routes.find(method + ":" + path);
    if(it != routes.end()) {
        return it->second;
    }
    return std::nullopt;
}
