#pragma once

#include "ynet/middleware.h"
#include <string>
#include <unordered_map>
namespace ynet {
    extern std::unordered_map<std::string, std::string> csrfs;
    Middleware Csrf();
}
