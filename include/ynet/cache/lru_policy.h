#pragma once

#include "ynet/cache/cache_policy.h"
namespace ynet {
    class LRUPolicy : public CachePolicy{
        private:
        public:
            void evict(std::unordered_map<std::string, CacheEntry>& entries) override;
    };
}
