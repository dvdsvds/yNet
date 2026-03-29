#pragma once

#include <ctime>
#include <optional>
#include <string>
#include <unordered_map>
namespace ynet {
    struct SessionData {
        time_t expires_at;
        std::unordered_map<std::string, std::string> data;
        std::string id;

        void set(const std::string& key, const std::string& value) { data[key] = value; }
        std::optional<std::string> get(const std::string& key);
    };
}
