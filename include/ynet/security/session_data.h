#pragma once

#include <ctime>
#include <atomic>
#include <optional>
#include <string>
#include <unordered_map>
namespace ynet {
    struct SessionData {
        std::atomic<time_t> expires_at;
        std::unordered_map<std::string, std::string> data;
        std::string id;

        SessionData() : expires_at(0) {}
        SessionData(SessionData&& other) noexcept : expires_at(other.expires_at.load()), data(std::move(other.data)), id(std::move(other.id)) {}
        SessionData& operator = (SessionData&& other) noexcept {
            expires_at.store(other.expires_at.load());
            data = std::move(other.data);
            id = std::move(other.id);
            return *this;
        }
        SessionData(const SessionData&) = delete;
        SessionData& operator=(const SessionData&) = delete;

        void set(const std::string& key, const std::string& value) { data[key] = value; }
        std::optional<std::string> get(const std::string& key);
    };
}
