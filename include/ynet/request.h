#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
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
        public:
            const std::string& getMethod() const { return method; }
            const std::string& getPath() const { return path; }
            const std::string& getVersion() const { return version; }
            const HeaderMap& getHeaders() const { return headers;}
            std::optional<std::string> getHeader(const std::string& key) const;
            const QueryMap& getQueryParams() const { return query_params; }
            std::optional<std::string> getQueryParam(const std::string& key) const;
            const std::string getClientIP() const { return client_ip; }
            const std::string& getBody() const { return body; }

            void setClientIP(const std::string& ip) { client_ip = ip; }

            static Request parse(const char* raw, size_t len);
    };
}
