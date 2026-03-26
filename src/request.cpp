#include <ynet/request.h>

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

        size_t start = 0;
        size_t amp = query_string.find('&');
        while(amp != std::string::npos) {
            std::string param = query_string.substr(start, amp - start);
            size_t eq = param.find('=');
            req.query_params[param.substr(0, eq)] = param.substr(eq + 1);
            start = amp + 1;
            amp = query_string.find('&', start);
        }
        std::string param = query_string.substr(start);
        size_t eq = param.find('=');
        req.query_params[param.substr(0, eq)] = param.substr(eq + 1);
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
    if(ct && ct->find("multipart/form-data") != std::string::npos) {
        MultipartParser parser;
        parser.parse(req.body.c_str(), req.body.size(), *ct);
        req.parts_ = parser.getParts();
    }
    return req;
}
