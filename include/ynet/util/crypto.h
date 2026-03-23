#pragma once

#include <string>
namespace ynet {
    std::string sha256(const std::string& input);
    std::string hmac_sha256(const std::string& key, const std::string& message);
    std::string random_hex(size_t length);
}
