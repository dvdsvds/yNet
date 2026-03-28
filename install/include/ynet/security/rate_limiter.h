#pragma once

#include "ynet/middleware.h"
namespace ynet {
    Middleware RateLimiter(int max_requests, int period_seconds);
}
