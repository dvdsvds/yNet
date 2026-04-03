#include <algorithm>
#include <cstring>
#include <string_view>
#include "ynet/util/multipart_parser.h"

using namespace ynet;

std::string MultipartParser::extractBoundary(const std::string& content_type) {
    size_t boundary_start = content_type.find("boundary");
    if(boundary_start == std::string::npos) return "";

    size_t eq = content_type.find('=', boundary_start);
    if(eq == std::string::npos) return "";

    std::string boundary;
    if(eq + 1 >= content_type.size()) return "";
    if(content_type[eq + 1] == '"') {
        size_t end = content_type.find('"', eq + 2);
        boundary = content_type.substr(eq + 2, end - (eq + 2));
    } else {
        boundary = content_type.substr(eq + 1);
    }

    return boundary;
}

void MultipartParser::parsePart(const char* raw, size_t len) {
    const char* sep = "\r\n\r\n";
    auto it = std::search(raw, raw + len, sep, sep + 4);
    if(it == raw + len) return;
    std::string header(raw, it);
    size_t filename_start = header.find("filename=\"");

    size_t name_start = header.find("name=\"");
    if(name_start == std::string::npos) return;

    Part part;

    if(filename_start != std::string::npos && name_start == filename_start + 4) {
        name_start = header.find("name=\"", filename_start + 10);
        if(name_start == std::string::npos) return;
    }

    size_t val_start = name_start + 6;
    size_t val_end = header.find('"', val_start);
    if(val_end == std::string::npos) return;
    part.name = header.substr(val_start, val_end - val_start);

    if(filename_start != std::string::npos) {
        size_t fval_start = filename_start + 10;
        size_t fval_end = header.find('"', fval_start);
        if(fval_end == std::string::npos) return;
        part.filename = header.substr(fval_start, fval_end - fval_start);
    } else {
        part.filename = "";
    }
    
    size_t content_type_start = header.find("Content-Type:");
    std::string content_type;
    if(content_type_start == std::string::npos) {
        part.content_type = "";
    } else {
        size_t val_start = content_type_start + 14;
        size_t semi = header.find("\r\n", val_start);
        part.content_type = header.substr(val_start, semi - val_start);
    }

    part.data = std::string_view(it + 4, len - (it + 4 - raw));
    if(!part.filename.empty() && part.data.empty()) {
        return;
    } 
    parts_.push_back(part);
}

void MultipartParser::parse(std::shared_ptr<const std::string> body, const std::string& content_type) {
    body_ = body;
    std::string boundary = extractBoundary(content_type);
    if(boundary.empty()) return;
    std::string sep = "--" + boundary;

    const char* cur = body_->data();
    size_t remaining = body_->size();

    const char* prev = nullptr;

    while(1) {
        auto it = std::search(cur, cur + remaining, sep.begin(), sep.end());
        if(it == cur + remaining) break;  

        if(prev != nullptr) {
            if(it - prev > 2) { 
                parsePart(prev, it - prev - 2);
            }
        }

        prev = it + sep.size() + 2;
        if(prev > body_->data() + body_->size()) break;
        cur = prev;
        remaining = body_->size() - (cur - body_->data());
    }
}

