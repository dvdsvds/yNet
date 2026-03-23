#pragma once

#include "ynet/middleware.h"
#include <vector>
namespace ynet {
    Middleware cors(std::vector<std::string> allowd_origins);
}
