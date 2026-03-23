#include <unordered_map>
#include <ynet/response.h>

using namespace ynet;

Response& Response::status(int code) {
    status_code_ = code;
    return *this;
}

Response& Response::header(const std::string& key, const std::string& value) {
    headers_[key] = value;
    return *this;
}

Response& Response::body(const std::string& body) {
    body_ = body;
    return * this;
}

std::optional<std::string> Response::getHeader(const std::string& key) const {
    auto it = headers_.find(key);
    if(it != headers_.end()) return it->second;
    return std::nullopt;
}

static const std::unordered_map<int, std::string> status_msg = {
    {200, "OK"},
    {201, "Created"},
    {204, "No Content"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {429, "Too Many Requests"},
    {500, "Internal Server Error"},
    {503, "Service Unavailable"}
};

std::string Response::build() {
    std::string result;
    headers_["Content-Length"] = std::to_string(body_.size());
    auto it = status_msg.find(status_code_);
    std::string msg = (it != status_msg.end()) ? it->second : "Unknown";
    result += "HTTP/1.1 " + std::to_string(status_code_) + " " + msg + "\r\n";

    for(auto& [key, value] : headers_) {
        result += key + ": " + value + "\r\n"; 
    }
    result += "\r\n";
    result += body_;
    return result;

}

void Response::send(Connection& conn) {
    std::string data = build();
    conn.write(data.c_str(), data.size());
}

