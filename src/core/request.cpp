#include "ynet/util/url.h"
#include "ynet/core/request.h"

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

Request Request::parse(const char* raw, size_t len) { 
    Request req;
    std::string data(raw, len);
    size_t sp1 = data.find(' ');
    size_t sp2 = data.find(' ', sp1 + 1);
    size_t line_end = data.find("\r\n");

    req.method = data.substr(0, sp1);
    req.path = data.substr(sp1 + 1, sp2 - sp1 - 1);
    size_t qpos = req.path.find('?');
    if(qpos != std::string::npos) {
        std::string query_string = req.path.substr(qpos + 1);
        req.path = req.path.substr(0, qpos);
        req.query_params = parseQuery(query_string);
    } 
    req.version = data.substr(sp2 + 1, line_end - sp2 - 1);

    size_t pos = line_end + 2;
    size_t next = data.find("\r\n", pos);
    while(next != pos) {
        std::string header = data.substr(pos, next - pos); 
        size_t colon = header.find(':');
        req.headers[header.substr(0, colon)] = header.substr(colon + 2);
        pos = next + 2;
        next = data.find("\r\n", pos);
    }

    req.body = data.substr(pos + 2);

    auto ct = req.getHeader("Content-Type");
    MultipartParser parser;
    if(ct && ct->find("multipart/form-data") != std::string::npos) {
        if(ct->find("boundary=") == std::string::npos) {
            req.parse_error = true;
            req.error_code = 400;
        } else {
            parser.parse(req.body.c_str(), req.body.size(), *ct);
            req.parts_ = parser.getParts();
            for(const auto& part : req.parts_) {
                if(part.filename.empty()) {
                    req.form_data[part.name] = std::string(part.data, part.data_len);
                }
            }
        }
    } else if(ct && ct->find("application/x-www-form-urlencoded") != std::string::npos) {
        req.form_data = parseQuery(req.body);
    }
    return req;
}
