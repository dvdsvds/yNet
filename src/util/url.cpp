#include <cctype>
#include <cstdio>
#include <iostream>
#include <ynet/util/url.h>

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
