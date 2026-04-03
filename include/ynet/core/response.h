#pragma once

#include <optional>
#include "ynet/cache/cache.h"
#include "ynet/core/request.h"
#include "ynet/net/connection.h"
#include "ynet/util/json.h"

namespace ynet {
    class Cache;
    class Response {
        private:
            int status_code_ = 200;
            HeaderMap headers_;
            std::string body_;
            Cache* cache_ = nullptr;
        public:
            static Response error(int code, const std::string& msg = "");
            int getStatusCode() const { return status_code_; }
            void setCache(Cache* c) { cache_ = c; }
            Response& status(int code);
            Response& header(const std::string& key, const std::string& value);
            Response& body(const std::string& body);
            Response& json(const std::string& json_str);
            Response& html(const std::string& html_str);
            Response& redirect(const std::string& url, int code = 302);
            Response& render(const std::string& file, const JsonValue& vars);
            std::optional<std::string> getHeader(const std::string& key) const;
            std::string build();
            void send(Connection& conn);
    };
}
