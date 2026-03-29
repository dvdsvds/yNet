#include "ynet/security/session_data.h"
#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/util/crypto.h"
#include <ctime>
#include <mutex>

namespace ynet {
    static std::unordered_map<std::string, SessionData> storage;
    static std::mutex mutex;

    std::optional<std::string> SessionData::get(const std::string& key) {
        auto it = data.find(key);
        if(it != data.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void create_session(Request& req, Response& res, int ttl, const std::string& cookie_name) {
        std::string id = random_hex(32);
        SessionData sd;
        sd.expires_at = std::time(nullptr) + ttl;
        sd.id = id;
        storage[id] = sd;
        req.session = sd;
        res.header("Set-Cookie", cookie_name + "=" + id);
    }

    Middleware Session(int ttl, const std::string& cookie_name, const std::string& cookie_flags) {
        return [ttl, cookie_name, cookie_flags](Request& req, Response& res, Next next) {
            std::optional<std::string> cookie_header = req.getHeader("Cookie");
            std::optional<std::string> session_id;
            std::string token;
            if(cookie_header.has_value()) {
                size_t i = 0;
                while(i < cookie_header.value().size()) {
                    size_t semi = cookie_header->find(';', i);
                    if(semi == std::string::npos) {
                        semi = cookie_header->size();
                    }
                    token = cookie_header->substr(i, semi - i);
                    i = semi + 1;
                    size_t eq = token.find('=');
                    if(eq == std::string::npos) continue;
                    std::string key = token.substr(0, eq);
                    std::string value = token.substr(eq + 1);
                    key.erase(0, key.find_first_not_of(' '));
                    if(key == cookie_name) {
                        session_id = value;
                        break;
                    }
                }

            }

            if(session_id.has_value()) {
                std::lock_guard<std::mutex> lock(mutex);
                auto it = storage.find(session_id.value());
                if(it == storage.end()) {
                    create_session(req, res, ttl, cookie_name);
                } else {
                    if(it->second.expires_at <= std::time(nullptr)) {
                        storage.erase(it);
                        create_session(req, res, ttl, cookie_name);
                    } else {
                        it->second.expires_at = std::time(nullptr) + ttl;
                        req.session = it->second;
                    }
                }
            } else {
                std::lock_guard<std::mutex> lock(mutex);
                create_session(req, res, ttl, cookie_name);
            }


            next();
            {
                std::lock_guard<std::mutex> lock(mutex);
                storage[req.session.id] = req.session;
            }
            std::optional<std::string> isheader = res.getHeader("Set-Cookie");
            if(isheader.has_value()) {
                res.header("Set-Cookie", isheader.value() + "; " + cookie_flags);
            }
        };
    }
}
