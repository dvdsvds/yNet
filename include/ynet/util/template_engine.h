#pragma once

#include <string>
#include "ynet/util/json.h"
#include "ynet/cache/cache.h"

namespace ynet {
    class TemplateEngine {
        private:
            Cache& cache;
            std::string renderString(const std::string& tmpl, const JsonValue& vars);
            std::string trim(const std::string& str);
            std::string jsonValueToString(const JsonValue& val);
            std::string htmlEscape(const std::string& str);
            bool isTruthy(const JsonValue& val);
        public:
            TemplateEngine(Cache& cache) : cache(cache) {}
            std::string render(const std::string& file_path, const JsonValue& vars);
    };
}
