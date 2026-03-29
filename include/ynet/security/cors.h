#pragma once

#include "ynet/core/middleware.h"
#include <vector>
namespace ynet {
    Middleware cors(std::vector<std::string> allowed_origins);
}
