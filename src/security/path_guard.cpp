#include "ynet/security/path_guard.h"

using namespace ynet;

PathGuard::PathGuard() : blacklist({}) {
    exact_paths = {
        "/wp-login.php", "/phpMyAdmin", "/pma",
        "/backup.sql", "/db.zip", "/swagger-ui.html", 
        "/v2/api-docs", "/docker-compose.yml", "/Dockerfile",
        "/package.json", "/.well-known/security.txt"
    };

    prefix_paths = {
        "/.env", "/.git", "/.svn", 
        "/.aws", "/.ssh", "/.vscode", 
        "/wp-admin", "/cgi-bin", "/actuator", 
        "/node_modules"
    };

    blocked_exts = {
        ".php", ".sql", ".bak", ".old", ".zip"
    };
}

void PathGuard::addExact(const std::string& path) {
    exact_paths.insert(path);
}

void PathGuard::addPrefix(const std::string& prefix) {
    prefix_paths.push_back(prefix);
}

void PathGuard::addExt(const std::string& ext) {
    blocked_exts.push_back(ext);
}

bool PathGuard::isBlocked(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mtx);
    return blacklist.count(ip) > 0;
}

bool PathGuard::check(const Request& req) {
    std::string ip = req.getClientIP();
    std::string path = req.getPath();

    if(isBlocked(ip)) return true;
    if(exact_paths.find(path) != exact_paths.end()) {
        std::lock_guard<std::mutex> lock(mtx);
        blacklist.insert(ip);
        return true;
    }
    for(const auto& prefix : prefix_paths) {
        if(path.compare(0, prefix.size(), prefix) == 0) {
            std::lock_guard<std::mutex> lock(mtx);
            blacklist.insert(ip);
            return true;
        }
    }
    for(const auto& ext : blocked_exts) {
        if(path.size() >= ext.size() && (path.compare(path.size() - ext.size(), ext.size(), ext) == 0)) {
            std::lock_guard<std::mutex> lock(mtx);
            blacklist.insert(ip);
            return true;
        }
    }
    return false;
}

Middleware PathGuard::toMiddleware() {
    return [this](Request& req, Response& res, Next next) {
        if(check(req)) {
            res.status(403);
            return;
        }
        next();
    };
}
