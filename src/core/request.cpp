#include "ynet/core/request.h"
#include "ynet/util/url.h"
#include <charconv>
#include <string>
#include <system_error>
#include <unordered_set>

using namespace ynet;

std::optional<std::string> Request::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    if(it != headers.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> Request::getQueryParam(const std::string& key) const {
    auto it = query_params.find(key);
    if(it != query_params.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> Request::getFormParam(const std::string& key) const {
    auto it = form_data.find(key);
    if(it != form_data.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> Request::getParam(const std::string& key) const {
    auto it = params.find(key);
    if(it != params.end()) {
        return it->second;
    }
    return std::nullopt;
}

Request Request::parse(const char* raw, size_t len, const Config& config) { 
    Request req;
    if(len > config.max_header_size + config.max_body_size) {
        req.parse_error = true;
        req.error_code = 413;
        return req;
    }

    std::string data(raw, len);
    size_t sp1 = data.find(' ');
    size_t sp2 = data.find(' ', sp1 + 1);
    size_t line_end = data.find("\r\n");
    size_t header_end = data.find("\r\n\r\n");
    if(header_end == std::string::npos) {
        req.parse_error = true;
        req.error_code = 400;
        return req;
    } else {
        if(header_end > config.max_header_size) {
            req.parse_error = true;
            req.error_code = 413;
            return req;
        }
        if(len - (header_end + 4) > config.max_body_size) {
            req.parse_error = true;
            req.error_code = 413;
            return req;
        }
    }

    if(sp1 == std::string::npos || sp2 == std::string::npos || line_end == std::string::npos) {
        req.parse_error = true;
        req.error_code = 400;
        return req;
    }

    req.method = data.substr(0, sp1);
    req.path = data.substr(sp1 + 1, sp2 - sp1 - 1);
    size_t qpos = req.path.find('?');
    if(qpos != std::string::npos) {
        std::string query_string = req.path.substr(qpos + 1);
        req.path = req.path.substr(0, qpos);
        req.query_params = parseQuery(query_string);
    } 

    req.version = data.substr(sp2 + 1, line_end - sp2 - 1);
    static const std::unordered_set<std::string> method = { "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS" };
    if(method.find(req.getMethod()) == method.end()) {
        req.parse_error = true;
        req.error_code = 400;
        return req;
    }

    static const std::unordered_set<std::string> version = { "HTTP/1.0", "HTTP/1.1" };
    if(version.find(req.getVersion()) == version.end()) {
        req.parse_error = true;
        req.error_code = 400;
        return req;
    }

    size_t pos = line_end + 2;
    size_t next = data.find("\r\n", pos);
    int header_count = 0;
    while(next != std::string::npos && next != pos) {
        std::string header = data.substr(pos, next - pos); 
        size_t colon = header.find(':');
        if(colon == std::string::npos) {
            pos = next + 2;
            next = data.find("\r\n", pos);
            continue;
        }

        std::string key = header.substr(0, colon);
        size_t value_start = header.find_first_not_of(" \t", colon + 1);
        if(value_start != std::string::npos) {
            req.headers[key] = header.substr(value_start);
        } else {
            req.headers[key] = "";
        }
        header_count++;
        if(header_count > config.max_headers) {
            req.parse_error = true;
            req.error_code = 431;
            return req;
        }

        pos = next + 2;
        next = data.find("\r\n", pos);
    }

    req.body = data.substr(pos + 2);
    auto cl = req.getHeader("Content-Length");
    size_t content_length = 0;
    if(cl) {
        auto result = std::from_chars(cl->data(), cl->data() + cl->size(), content_length);
        if(result.ec != std::errc()) {
            req.parse_error = true;
            req.error_code = 400;
            return req;
        } else {
            if(content_length != req.body.size()) {
                req.parse_error = true;
                req.error_code = 400;
                return req;
            }
        }
    }
    auto ct = req.getHeader("Content-Type");
    MultipartParser parser;
    if(ct && ct->find("multipart/form-data") != std::string::npos) {
        if(ct->find("boundary=") == std::string::npos) {
            req.parse_error = true;
            req.error_code = 400;
        } else {
            auto body_ptr = std::make_shared<const std::string>(req.body);
            parser.parse(body_ptr, *ct);
            req.parts_ = parser.getParts();
            for(const auto& part : req.parts_) {
                if(part.filename.empty()) {
                    req.form_data[part.name] = std::string(part.data);
                }
            }
        }
    } else if(ct && ct->find("application/x-www-form-urlencoded") != std::string::npos) {
        req.form_data = parseQuery(req.body);
    }
    return req;
}
