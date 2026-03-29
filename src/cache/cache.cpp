#include <fstream>
#include <filesystem>
#include "ynet/cache/cache.h"

using namespace ynet;

void Cache::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    entries.clear();
}

std::string Cache::loadFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mtx);
    auto current_mtime = fs::last_write_time(filepath);
    auto it = entries.find(filepath);

    if(it != entries.end() && it->second.mtime == current_mtime) {
        it->second.last_access = std::chrono::steady_clock::now();
        return it->second.content;
    }

    if(entries.size() >= max_entries) policy->evict(entries);
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    entries[filepath] = {content, current_mtime, std::chrono::steady_clock::now()};
    return content;
}
