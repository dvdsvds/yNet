#include <cctype>
#include <cstdio>
#include <iostream>
#include <vector>
#include "ynet/util/url.h"

namespace ynet {
    std::string urlEncode(const std::string& str) {
        size_t i = 0;
        std::string result;
        while(i < str.size()) {
            if(std::isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~') {
                result += str[i];
            } else {
                char buf[4];
                std::snprintf(buf, 4, "%%%02X", static_cast<unsigned char>(str[i]));
                result += buf;
            }
            i++;
        }
        return result;
    }

    std::string urlDecode(const std::string& str) {
        size_t i = 0;
        std::string result;
        while(i < str.size()) {
            if(str[i] == '%') {
                if(i + 2 < str.size()) {
                    std::string ch = str.substr(i + 1, 2);
                    try {
                        result += static_cast<char>(std::stoi(ch, nullptr, 16));
                        i += 3;
                    } catch(std::exception& e) {
                        result += str[i];    
                        i++;
                        std::cerr << "[ynet] urlDecode: invalid hex: " << e.what() << std::endl;
                    }
                } else {
                    result += str[i];
                    i++;
                }
            } else if(str[i] == '+') {
                result += ' ';
                i++;
            } else {
                result += str[i];
                i++;
            }
        }
        return result;
    }

    bool isDoubleEncoded(const std::string& decoded_str) {
        if(decoded_str.find('\0') != std::string::npos) return true;

        size_t pos = 0;
        while ((pos = decoded_str.find('%', pos)) != std::string::npos) {
            if(pos + 2 < decoded_str.size() && std::isxdigit(decoded_str[pos + 1]) && std::isxdigit(decoded_str[pos + 2])) {
                return true;
            } 
            pos++;
        }
        return false;
    }

    std::string normalizePath(const std::string& decoded_path) {
        std::vector<std::string> seg;
        size_t start = 0;
        size_t pos = 0;
        while((pos = decoded_path.find('/', start)) != std::string::npos) {
            std::string token = decoded_path.substr(start, pos - start);
            if(token.empty() || token == ".") {
            } else if(token == "..") {
                if(!seg.empty()) seg.pop_back();
            } else {
                seg.push_back(token);
            }
            start = pos + 1;
        }
        std::string token = decoded_path.substr(start);
        if(token.empty() || token == ".") {
        } else if(token == "..") {
            if(!seg.empty()) seg.pop_back();
        } else {
            seg.push_back(token);
        }

        std::string result;
        for(const auto& s : seg) {
            result += "/" + s;
        }

        return result.empty() ? "/" : result;
    }

    std::unordered_map<std::string, std::string> parseQuery(const std::string& query) {
        size_t pos = 0;
        std::unordered_map<std::string, std::string> q;
        while(pos < query.size()) {
            std::string token;
            size_t found = query.find('&', pos);
            if(found == std::string::npos) {
                token = query.substr(pos);
                pos = query.size();
            } else {
                token = query.substr(pos, found - pos);
                pos = found + 1;
            }

            size_t eq = token.find('=');
            if(eq == std::string::npos) {
                q[urlDecode(token)] = "";
            } else {
                q[urlDecode(token.substr(0, eq))] = urlDecode(token.substr(eq + 1));
            }
        }
        return q;
    }
}
