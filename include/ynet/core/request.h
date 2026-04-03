#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "ynet/core/config.h"
#include "ynet/security/session_data.h"
#include "ynet/util/multipart_parser.h"

namespace ynet {
    using HeaderMap = std::unordered_map<std::string, std::string>;
    using QueryMap = std::unordered_map<std::string, std::string>;
    class Request {
        private:
            std::string method;
            std::string path;
            std::string version;
            HeaderMap headers;
            std::string body;
            std::string client_ip;
            QueryMap query_params;
            std::vector<Part> parts_;
            std::string csrf_token;
            std::unordered_map<std::string, std::string> form_data;
            bool parse_error = false;
            int error_code = 0;
            std::unordered_map<std::string, std::string> params;
        public:
            std::shared_ptr<SessionData> session;
            const std::string& getMethod() const { return method; }
            const std::string& getPath() const { return path; }
            const std::string& getVersion() const { return version; }
            const HeaderMap& getHeaders() const { return headers;}
            std::optional<std::string> getHeader(const std::string& key) const;
            const QueryMap& getQueryParams() const { return query_params; }
            std::optional<std::string> getQueryParam(const std::string& key) const;
            const std::string& getClientIP() const { return client_ip; }
            const std::string& getBody() const { return body; }
            const std::vector<Part>& getParts() const { return parts_; }
            const std::string& getCsrfToken() const { return csrf_token; }
            bool isParseError() const { return parse_error; }
            int getErrorCode() const { return error_code; }
            void setClientIP(const std::string& ip) { client_ip = ip; }
            void setCsrfToken(const std::string& token) { csrf_token = token; }
            void setParam(const std::string& key, const std::string& value) { params[key] = value; }

            std::optional<std::string> getParam(const std::string& key) const;
            std::optional<std::string> getFormParam(const std::string& key) const;
            static Request parse(const char* raw, size_t len, const Config& config);
    };
}
