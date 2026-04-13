#pragma once

#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace ynet {
    class PathGuard {
        private:
            std::unordered_set<std::string> exact_paths;
            std::vector<std::string> prefix_paths;
            std::vector<std::string> blocked_exts;
            std::unordered_set<std::string> blacklist;
            std::mutex mtx;
            std::unordered_set<std::string> whitelist;

        public:
            PathGuard();
            void addExact(const std::string& path);
            void addPrefix(const std::string& prefix);
            void addExt(const std::string& ext);
            bool isBlocked(const std::string& ip);
            bool check(const Request& req);
            Middleware toMiddleware();
            void addWhitelist(const std::string& ip);

    };
}
