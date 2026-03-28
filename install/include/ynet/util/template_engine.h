#pragma once

#include <ctime>
#include <string>
#include <unordered_map>
#include <mutex>
#include "ynet/util/json.h"

namespace ynet {
    struct CacheEntry {
        std::string content;
        time_t mtime;
    };

    class TemplateEngine {
        private:
            std::unordered_map<std::string, CacheEntry> cacheMap;
            std::mutex ctx;
            std::string loadFile(const std::string& file_path);
            std::string renderString(const std::string& tmpl, const JsonValue& vars);

            std::string trim(const std::string& str);
            std::string jsonValueToString(const JsonValue& val);
            std::string htmlEscape(const std::string& str);
            bool isTruthy(const JsonValue& val);
        public:
            std::string render(const std::string& file_path, const JsonValue& vars);
            void clearCache();
    };
}
