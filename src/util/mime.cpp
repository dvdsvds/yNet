#include <unordered_map>
#include "ynet/util/mime.h"

namespace ynet {
    static const std::unordered_map<std::string, std::string> m = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".txt", "text/plain"},
        {".csv", "text/csv"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".webp", "image/webp"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".gz", "application/gzip"},
        {".wasm", "application/wasm"},
        {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"}
    };

    std::string getMimeType(const std::string& path) {
        size_t rcomma = path.rfind('.');
        if(rcomma != std::string::npos) {
            std::string ext = path.substr(rcomma);
            auto it = m.find(ext);
            if(it != m.end()) {
                return it->second;
            }
        }
        return "application/octet-stream";
    }
}
