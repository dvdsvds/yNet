#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include "ynet/util/json.h"

namespace ynet {
    static JsonValue parseValue(const std::string& s, size_t& i, int depth = 0);

    static void skipWhitespace(const std::string& s, size_t& i) {
        while(i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) {
            i++;
        } 
    }

    static JsonValue parseString(const std::string& s, size_t& i) {
        i++;
        std::string data;
        while(i < s.size() && (s[i] != '"')) {
            if(s[i] == '\\') {
                i++;
                if(i >= s.size()) throw std::runtime_error("unexpected end of string");
                switch(s[i]) {
                    case '"': data += '"'; break;
                    case '\\': data += '\\'; break;
                    case '/': data += '/'; break;
                    case 'n': data += '\n'; break;
                    case 'r': data += '\r'; break;
                    case 't': data += '\t'; break;
                    case 'b': data += '\b'; break;
                    case 'f': data += '\f'; break;
                    case 'u': {
                        if(i + 4 >= s.size()) throw std::runtime_error("invalid unicode escape");
                        std::string hex = s.substr(i + 1, 4);
                        uint16_t cp = std::stoul(hex, nullptr, 16);
                        if(cp < 0x80) {
                            data += static_cast<char>(cp);
                        } else if(cp < 0x800) {
                            data += static_cast<char>(0xC0 | (cp >> 6));
                            data += static_cast<char>(0x80 | (cp & 0x3F));
                        } else {
                            data += static_cast<char>(0xE0 | (cp >> 12));
                            data += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            data += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                        i += 4;
                        break;
                    }
                    default: throw std::runtime_error("invalid escape character");
                }
                i++;
                continue;
            }
            data += s[i];
            i++;        }
        i++;
        return JsonValue{data};
    }

    static JsonValue parseNumber(const std::string& s, size_t& i) {
        std::string data;
        while(i < s.size() && (std::isdigit(s[i]) || s[i] == '-' || s[i] == '.')) {
            data += s[i]; 
            i++;
        }

        if(data.empty()) throw std::runtime_error("invalid number");
        int dots = 0, dashes = 0;
        for(size_t j = 0; j < data.size(); j++) {
            if(data[j] == '-') {
                if(j != 0) throw std::runtime_error("invalid number");
                dashes++;
            }
            if(data[j] == '.') dots++;
        }
        if(dots > 1 || dashes > 1) throw std::runtime_error("invalid number");

        if(data.find(".") != std::string::npos) {
           return JsonValue{std::stod(data)};
        } else {
            return JsonValue{std::stoll(data)};
        }
    }

    static JsonValue parseArray(const std::string& s, size_t& i, int depth) {
        i++;
        std::vector<JsonValue> datas;
        while(i < s.size() && (s[i] != ']')) {
            datas.push_back(parseValue(s, i, depth + 1));     
            skipWhitespace(s, i);
            if(s[i] == ',') i++;
            skipWhitespace(s, i);
        }
        if(i >= s.size()) throw std::runtime_error("unexpected end of input");
        i++;
        return JsonValue{datas};
    }

    static JsonValue parseObject(const std::string& s, size_t& i, int depth) {
        i++;
        std::map<std::string, JsonValue> datas;
        while(i < s.size() && (s[i] != '}')) {
            skipWhitespace(s, i);
            if(i >= s.size() || s[i] != '"') throw std::runtime_error("expected string key");
            std::string key = std::get<std::string>(parseString(s, i).data);
            skipWhitespace(s, i);
            if(s[i] == ':') i++;
            skipWhitespace(s, i);
            JsonValue value = parseValue(s, i, depth + 1);
            datas[key] = value;
            skipWhitespace(s, i);
            if(s[i] == ',') i++;
            skipWhitespace(s, i);
        }
        if(i >= s.size()) throw std::runtime_error("unexpected end of input");
        i++;
        return JsonValue{datas};
    }

    static JsonValue parseValue(const std::string& s, size_t& i, int depth) {
        if(depth > 64) throw std::runtime_error("max nesting depth exceeded");
        if(i >= s.size()) throw::std::runtime_error("unexpected end of input");
        if(s[i] == '"') {
            return parseString(s, i);
        } else if(std::isdigit(s[i]) || s[i] == '-') {
            return parseNumber(s, i);
        } else if(s[i] == '[') {
            return parseArray(s, i, depth);
        } else if(s[i] == '{') {
            return parseObject(s, i, depth);
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
        auto result = parseValue(json, i);
        skipWhitespace(json, i);
        if(i != json.size()) throw std::runtime_error("trailing characters");
        return result;
    }

    static std::string escapeString(const std::string& s) {
        std::string result = "\"";
        for(char c : s) {
            switch(c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                default: result += c;
            }
        }
        result += "\"";
        return result;
    }

    std::string stringify(const JsonValue& val) {
        return std::visit([](auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
            else if constexpr (std::is_same_v<T, bool>) return v ? "true" : "false";
            else if constexpr (std::is_same_v<T, int64_t>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, double>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, std::string>) return escapeString(v);
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
                    result += escapeString(key) + ":" + stringify(value);
                    first = false;
                }
                result += "}";
                return result;
            }
        }, val.data);
    }

    std::string toText(const JsonValue &val) {
        return std::visit([](auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) return "";
            else if constexpr (std::is_same_v<T, bool>) return v ? "true" : "false";
            else if constexpr (std::is_same_v<T, int64_t>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, double>) return std::to_string(v);
            else if constexpr (std::is_same_v<T, std::string>) return v;
            else if constexpr (std::is_same_v<T, std::vector<JsonValue>>) return "";
            else if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) return "";
        }, val.data);
    }
}
