#pragma once

#include "ynet/config.h"
namespace ynet {
    Config loadConfig();
    size_t parseSize(const std::string& val);
}
