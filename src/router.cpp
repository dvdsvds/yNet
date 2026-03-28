#include "ynet/server.h"
#include <fstream>
#include <ynet/router.h>

using namespace ynet;

void Route::html(const std::string& content) {
    router.addRoute(method, path, [content](Request&, Response& res) {
        res.html(content);
    });
}

void Route::json(const std::string& content) {
    router.addRoute(method, path, [content](Request&, Response& res) {
        res.json(content);
    });
}

void Route::file(const std::string& filepath) {
    router.addRoute(method, path, [&router = this->router, filepath](Request&, Response& res) {
        std::string content = router.loadFile(filepath);
        res.html(content);
    });
}

void Route::handle(Handler fn) {
    router.addRoute(method, path, fn);
}

void Router::addRoute(const std::string& method, const std::string& path, Handler handler) {
    routes[method + ":" + path] = handler;
}

void ErrorRoute::html(const std::string& content) {
    server.onError(code, [content](const Request&) {
        Response res;
        res.html(content);
        return res;
    });
}

void ErrorRoute::json(const std::string& content) {
    server.onError(code, [content](const Request&) {
        Response res;
        res.json(content);
        return res;
    });
}

void ErrorRoute::file(const std::string& filepath) {
    server.onError(code, [filepath](const Request&) {
        std::ifstream f(filepath);
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        Response res;
        res.html(content);
        return res;
    });
}

Route Router::get(const std::string& path) {
    return Route(*this, "GET", path);
}

Route Router::post(const std::string& path) {
    return Route(*this, "POST", path);
}

Route Router::put(const std::string& path) {
    return Route(*this, "PUT", path);
}

Route Router::del(const std::string& path) {
    return Route(*this, "DELETE", path);
}

std::optional<Handler> Router::resolve(const std::string& method, const std::string& path) const {
    auto it = routes.find(method + ":" + path);
    if(it != routes.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string Router::loadFile(const std::string& filepath) {
    auto current_mtime = fs::last_write_time(filepath);
    auto it = file_cache.find(filepath);

    if(it != file_cache.end() && it->second.mtime == current_mtime) {
        return it->second.content;
    }

    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file_cache[filepath] = {content, current_mtime};
    return content;
}
