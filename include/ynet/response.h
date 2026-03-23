#pragma once

#include <optional>
#include <ynet/request.h>
#include <ynet/net/connection.h>

namespace ynet {
    class Response {
        private:
            int status_code_;
            HeaderMap headers_;
            std::string body_;
        public:
            Response& status(int code);
            Response& header(const std::string& key, const std::string& value);
            Response& body(const std::string& body);

            std::optional<std::string> getHeader(const std::string& key) const;
            std::string build();
            void send(Connection& conn);

            int getStatusCode() const { return status_code_; }
    };
}
