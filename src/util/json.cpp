#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include "ynet/util/json.h"

namespace ynet {
    static JsonValue parseValue(const std::string& s, size_t& i);

    static void skipWhitespace(const std::string& s, size_t& i) {
        while(i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) {
            i++;
        } 
    }

    static JsonValue parseString(const std::string& s, size_t& i) {
        i++;
        std::string data;
        while(i < s.size() && (s[i] != '"')) {
            data += s[i]; 
            i++;
        }
        i++;
        return JsonValue{data};
    }

    static JsonValue parseNumber(const std::string& s, size_t& i) {
        std::string data;
        while(i < s.size() && (std::isdigit(s[i]) || s[i] == '-' || s[i] == '.')) {
            data += s[i]; 
            i++;
        }

        if(data.find(".") != std::string::npos) {
           return JsonValue{std::stod(data)};
        } else {
            return JsonValue{std::stoll(data)};
        }
    }

    static JsonValue parseArray(const std::string& s, size_t& i) {
        i++;
        std::vector<JsonValue> datas;
        while(i < s.size() && (s[i] != ']')) {
            datas.push_back(parseValue(s, i));     
            skipWhitespace(s, i);
            if(s[i] == ',') i++;
            skipWhitespace(s, i);
        }
        i++;
        return JsonValue{datas};
    }

    static JsonValue parseObject(const std::string& s, size_t& i) {
        i++;
        std::map<std::string, JsonValue> datas;
        while(i < s.size() && (s[i] != '}')) {
            skipWhitespace(s, i);
            std::string key = std::get<std::string>(parseString(s, i).data);
            skipWhitespace(s, i);
            if(s[i] == ':') i++;
            skipWhitespace(s, i);
            JsonValue value = parseValue(s, i);
            datas[key] = value;
            skipWhitespace(s, i);
            if(s[i] == ',') i++;
            skipWhitespace(s, i);
        }
        i++;
        return JsonValue{datas};
    }

    JsonValue parseValue(const std::string& s, size_t& i) {
        if(s[i] == '"') {
            return parseString(s, i);
        } else if(std::isdigit(s[i]) || s[i] == '-') {
            return parseNumber(s, i);
        } else if(s[i] == '[') {
            return parseArray(s, i);
        } else if(s[i] == '{') {
            return parseObject(s, i);
        } else if(s[i] == 't' || s[i] == 'f') {
            if(s[i] == 't') { i += 4; return JsonValue(true); }
            else {i += 5; return JsonValue(false); }
        } else if(s[i] == 'n') {
            i += 4;
            return JsonValue{nullptr};
        } else {
            throw std::runtime_error("unexpected character");
        }
    }

    JsonValue parse(const std::string& json) {
        size_t i = 0;
        skipWhitespace(json, i);
        return parseValue(json, i);
    }

    std::string stringify(const JsonValue& val) {
        return std::visit([](auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
            else if constexpr (std::is_same_v<T, bool>) return v ? "true" : "false";
            else if constexpr (std::is_same_v<T, int64_t>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, double>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, std::string>) return "\"" + v + "\"";
            else if constexpr (std::is_same_v<T, std::vector<JsonValue>>) {
                std::string result = "[";
                for(size_t j = 0; j < v.size(); j++) {
                    result += stringify(v[j]);
                    if(j + 1 < v.size()) result += ",";
                }
                result += "]";
                return result;
            }
            else if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) {
                std::string result = "{";
                bool first = true;
                for(auto& [key, value] : v) {
                    if(!first) result += ",";
                    result += "\"" + key + "\":" + stringify(value);
                    first = false;
                }
                result += "}";
                return result;
            }
        }, val.data);
    }

}
