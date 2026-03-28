#include <algorithm>
#include <cstring>
#include <ynet/util/multipart_parser.h>

using namespace ynet;

std::string MultipartParser::extractBoundary(const std::string& content_type) {
    size_t boundary_start = content_type.find("boundary");
    if(boundary_start == std::string::npos) return "";

    size_t eq = content_type.find('=', boundary_start);
    if(eq == std::string::npos) return "";

    std::string boundary;
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
    std::string header(raw, it);
    size_t name_start = header.find("name=\"");

    Part part;
    size_t val_start = name_start + 6;
    size_t val_end = header.find('"', val_start);
    std::string val = header.substr(val_start, val_end - val_start);

    if(header[name_start - 1] != 'e') {
        part.name = val;
        size_t filename_start = header.find("filename=\"", val_end);
        if(filename_start != std::string::npos) {
            size_t val_start2 = filename_start + 10;
            size_t val_end2 = header.find('"', val_start2);
            part.filename = header.substr(val_start2, val_end2 - val_start2);
        } else {
            part.filename = "";
        }
    } else {
        part.filename = val;
        size_t name_start2 = header.find("name=\"", val_end);
        size_t val_start2 = name_start2 + 6;
        size_t val_end2 = header.find('"', val_start2);
        part.name = header.substr(val_start2, val_end2 - val_start2);
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

    part.data = it + 4;
    part.data_len = len - (part.data - raw);

    if(!part.filename.empty() && part.data_len == 0) {
        return;
    } 
    parts_.push_back(part);
}

void MultipartParser::parse(const char* body, size_t body_len, const std::string& content_type) {
    std::string boundary = extractBoundary(content_type);
    std::string sep = "--" + boundary;

    const char* cur = body;
    size_t remaining = body_len;

    const char* prev = nullptr;

    while(1) {
        auto it = std::search(cur, cur + remaining, sep.begin(), sep.end());
        if(it == cur + remaining) break;  

        if(prev != nullptr) {
            parsePart(prev, it - prev - 2);
        }

        prev = it + sep.size() + 2;
        cur = prev;
        remaining = body_len - (cur - body);
    }
}

