#pragma once

#include <optional>
#include "ynet/core/request.h"
#include "ynet/net/connection.h"

namespace ynet {
    class Response {
        private:
            int status_code_;
            HeaderMap headers_;
            std::string body_;
        public:
            static Response error(int code, const std::string& msg = "");
            Response& status(int code);
            Response& header(const std::string& key, const std::string& value);
            Response& body(const std::string& body);
            Response& json(const std::string& json_str);
            Response& html(const std::string& html_str);
            Response& redirect(const std::string& url, int code = 302);

            std::optional<std::string> getHeader(const std::string& key) const;
            std::string build();
            void send(Connection& conn);

            int getStatusCode() const { return status_code_; }
    };
}
