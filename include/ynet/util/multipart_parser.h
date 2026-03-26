#pragma once

#include <string>
#include <vector>
namespace ynet {
    struct Part {
        std::string name;
        std::string filename;
        std::string content_type;
        const char* data;
        size_t data_len;
    };

    class MultipartParser {
        private:
            std::string boundary_;
            std::vector<Part> parts_;
            std::string extractBoundary(const std::string& content_type);
            void parsePart(const char* raw, size_t len);
        public:
            void parse(const char* body, size_t body_len, const std::string& content_type);
            const std::vector<Part>& getParts() const { return parts_; }
    };
}
