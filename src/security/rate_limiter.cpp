#include <chrono>
#include <memory>
#include <unordered_map>
#include <ynet/security/rate_limiter.h>

namespace ynet {
    struct Bucket {
        double tokens = 0;
        std::chrono::steady_clock::time_point last_refill = std::chrono::steady_clock::now();
        bool initialized = false;
    };
    
    Middleware RateLimiter(int max_requests, int period_seconds) {
        auto buckets = std::make_shared<std::unordered_map<std::string, Bucket>>();
        return [buckets, max_requests, period_seconds](const Request& req, Response& res, Next next) {
            std::string ip = req.getClientIP();
            auto now = std::chrono::steady_clock::now();
            auto& bucket = (*buckets)[ip];
            if(!bucket.initialized) {
                bucket.tokens = max_requests;
                bucket.last_refill = now;
                bucket.initialized = true;
            }
            double elapsed = std::chrono::duration<double>(now - bucket.last_refill).count();
            bucket.tokens += elapsed * (static_cast<double>(max_requests) / period_seconds);
            if(bucket.tokens > max_requests) bucket.tokens = max_requests;
            bucket.last_refill = now;
            if(bucket.tokens >= 1) {
                bucket.tokens -= 1;
                next();
            } else {
                res.status(429).body("Too Many Requests");
            }
        };
    }
}
