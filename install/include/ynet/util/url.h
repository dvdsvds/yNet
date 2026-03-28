#pragma once

#include <string>
#include <unordered_map>
namespace ynet {
    std::string urlEncode(const std::string& str);
    std::string urlDecode(const std::string& str);
    std::unordered_map<std::string, std::string> parseQuery(const std::string& query);
}
