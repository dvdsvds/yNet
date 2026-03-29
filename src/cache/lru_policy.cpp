#include "ynet/cache/lru_policy.h"

using namespace ynet;

void LRUPolicy::evict(std::unordered_map<std::string, CacheEntry>& entries) {
    auto oldest = entries.begin();
    for(auto it = entries.begin(); it != entries.end(); ++it) {
        if(it->second.last_access < oldest->second.last_access) {
            oldest = it;
        }
    }
    entries.erase(oldest);    
}
