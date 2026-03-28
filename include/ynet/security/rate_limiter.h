#pragma once

#include "ynet/core/middleware.h"
namespace ynet {
    Middleware RateLimiter(int max_requests, int period_seconds);
}
