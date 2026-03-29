#pragma once

#include "ynet/core/middleware.h"
namespace ynet {
    Middleware Session(int ttl = 3600, const std::string& cookie_name = "session_id", const std::string& cookie_flags = "HttpOnly; Secure; SameSite=Strict");
}
