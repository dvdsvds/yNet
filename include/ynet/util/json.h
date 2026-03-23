#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace ynet {
    struct JsonValue {
        std::variant<
            std::nullptr_t,
            bool,
            int64_t,
            double,
            std::string,
            std::vector<JsonValue>,
            std::map<std::string, JsonValue>
        > data; 
    };
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    JsonValue parse(const std::string& json);
    std::string stringify(const JsonValue& val);
}
