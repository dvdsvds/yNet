#pragma once

#include <string>
#include <unordered_map>
#include "ynet/cache/cache_entry.h"
namespace ynet {
    class CachePolicy {
        public:
            virtual void evict(std::unordered_map<std::string, CacheEntry>& entries) = 0;
            virtual ~CachePolicy() = default;
    };
}
