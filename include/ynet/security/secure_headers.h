#pragma once
#include <string>
#include "ynet/core/middleware.h"

namespace ynet {
    Middleware SecureHeaders(const std::string& csp = "default-src 'self'; style-src 'self' 'unsafe-inline'; script-src 'self' 'unsafe-inline'");
}
