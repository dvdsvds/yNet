#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
namespace ynet {
    struct Part {
        std::string name;
        std::string filename;
        std::string content_type;
        std::string_view data;
    };

    class MultipartParser {
        private:
            std::shared_ptr<const std::string> body_;
            std::string boundary_;
            std::vector<Part> parts_;
            std::string extractBoundary(const std::string& content_type);
            void parsePart(const char* raw, size_t len);
        public:
            void parse(std::shared_ptr<const std::string> body,  const std::string& content_type);
            const std::vector<Part>& getParts() const { return parts_; }
    };
}
