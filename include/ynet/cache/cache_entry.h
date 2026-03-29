#pragma once

#include <string>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace ynet {
    struct CacheEntry {
        std::string content;
        fs::file_time_type mtime;
        std::chrono::steady_clock::time_point last_access;
    };
}
