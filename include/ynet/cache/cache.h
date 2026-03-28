#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <filesystem>
#include <unordered_map>
namespace fs = std::filesystem;
namespace ynet {
    struct CacheEntry {
        std::string content;
        fs::file_time_type mtime;
        std::chrono::steady_clock::time_point last_access;
    };

    class Cache {
        private:
            std::unordered_map<std::string, CacheEntry> entries;
            std::mutex mtx;
            size_t max_entries;
            void evict();
        public:
            Cache(size_t max_entries = 1024) : max_entries(max_entries) {}
            std::string loadFile(const std::string& filepath);
            void clear();
    };
}
