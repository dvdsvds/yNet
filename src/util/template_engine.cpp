#include <cstddef>
#include <fstream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include "ynet/util/template_engine.h"

using namespace ynet;

std::string TemplateEngine::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");

    return str.substr(first, last - first + 1);
}

std::string TemplateEngine::jsonValueToString(const JsonValue& val) {
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


std::string TemplateEngine::htmlEscape(const std::string& str) {
    std::string result;
    for(const char& c : str) {
        switch(c) {
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&#39;";
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

bool TemplateEngine::isTruthy(const JsonValue& val) {
    return std::visit([](auto& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) return false;
        else if constexpr (std::is_same_v<T, bool>) return v;
        else if constexpr (std::is_same_v<T, int64_t>) return v != 0;
        else if constexpr (std::is_same_v<T, double>) return v != 0.0;
        else if constexpr (std::is_same_v<T, std::string>) return !v.empty();
        else if constexpr (std::is_same_v<T, std::vector<JsonValue>>) return !v.empty();
        else if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) return !v.empty();
    }, val.data);
}

#include <iostream>
std::string TemplateEngine::loadFile(const std::string& file_path) {
    struct stat st;
    if(stat(file_path.c_str(), &st) == -1) {
        std::cout << "stat failed: " << file_path << std::endl;
        return "";
    }
    if(stat(file_path.c_str(), &st) == -1) {
        return "";
    }
    std::lock_guard<std::mutex> lock(ctx);

    auto it = cacheMap.find(file_path);
    if(it != cacheMap.end()) {
        if(it->second.mtime == st.st_mtime) {
            return it->second.content;
        };
    } 

    std::ifstream file(file_path);
    if(!file.is_open()) return "";

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    cacheMap[file_path] = {content, st.st_mtime};

    return content;
}

std::string TemplateEngine::renderString(const std::string& tmpl, const JsonValue& vars) {
    size_t pos = 0;
    std::string result;
    while(1) {
        size_t start = tmpl.find("{{", pos);
        if(start == std::string::npos) break;
        result += tmpl.substr(pos, start - pos);

        if(start + 2 < tmpl.size() && tmpl[start + 2] == '{') {
            size_t end = tmpl.find("}}}", start);
            if(end == std::string::npos) break;
            std::string key_name = tmpl.substr(start + 3, end - start - 3);
            auto& obj = std::get<Object>(vars.data);
            auto var = obj.find(trim(key_name));
            if(var != obj.end()) {
                result += jsonValueToString(var->second);
            }
            pos = end + 3;
        } else {
            size_t end = tmpl.find("}}", start);
            if(end == std::string::npos) break;
            std::string key_name = tmpl.substr(start + 2, end - start - 2);
            auto& obj = std::get<Object>(vars.data);
            if(key_name.starts_with("#if ")) {
                std::string var_name = trim(key_name.substr(4));
                size_t if_end = tmpl.find("{{/if}}", end + 2);
                if(if_end == std::string::npos) break;

                std::string insideBlock = tmpl.substr(end + 2, if_end - end - 2);
                auto var = obj.find(var_name);
                if(var != obj.end() && isTruthy(var->second)) {
                    result += renderString(insideBlock, vars);
                }
                pos = if_end + 7;
            } else if(key_name.starts_with("#each ")) {
                std::string var_name = trim(key_name.substr(6));
                size_t each_end = tmpl.find("{{/each}}", end + 2);
                if(each_end == std::string::npos) break;

                std::string insideBlock = tmpl.substr(end + 2, each_end - end - 2);
                auto var = obj.find(var_name);
                if(var != obj.end()) {
                    auto& arr = std::get<Array>(var->second.data);
                    for(auto& item : arr) {
                        result += renderString(insideBlock, item);
                    }
                }
                pos = each_end + 9;
            } else if(key_name.starts_with("> ")) {
                std::string file_name = trim(key_name.substr(2));
                std::string file = loadFile(file_name);
                result += renderString(file, vars);
                pos = end + 2;
            } else {
                auto var = obj.find(trim(key_name));
                if(var != obj.end()) {
                    result += htmlEscape(jsonValueToString(var->second));
                }
                pos = end + 2;
            }
        }
    }
    result += tmpl.substr(pos);
    return result;
}

std::string TemplateEngine::render(const std::string& file_path, const JsonValue& vars) {
    return renderString(loadFile(file_path), vars);
}

void TemplateEngine::clearCache() {
    std::lock_guard<std::mutex> lock(ctx);
    cacheMap.clear();
}
