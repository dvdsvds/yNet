#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "ynet/cache/cache_policy.h"
#include "ynet/cache/lru_policy.h"

namespace ynet {
    class Cache {
        private:
            std::unordered_map<std::string, CacheEntry> entries;
            std::mutex mtx;
            size_t max_entries;
            std::unique_ptr<CachePolicy> policy;
        public:
            Cache(size_t max_entries = 1024, std::unique_ptr<CachePolicy> policy = std::make_unique<LRUPolicy>()) : max_entries(max_entries),  policy(std::move(policy)){}
            std::string loadFile(const std::string& filepath);
            void clear();
    };
}
