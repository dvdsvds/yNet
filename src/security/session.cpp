#include "ynet/security/session_data.h"
#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/util/crypto.h"
#include <condition_variable>
#include <ctime>
#include <mutex>
#include <shared_mutex>

namespace ynet {
    static std::atomic<bool> stop_flag{false};
    static std::condition_variable cv;
    static std::thread cleanup_thread;
    static std::unordered_map<std::string, std::shared_ptr<SessionData>> storage;
    static std::shared_mutex mutex;
    static std::mutex cleanup_mutex;

    void cleanup() {
        while(!stop_flag) {
            std::unique_lock<std::mutex> clock(cleanup_mutex);
            cv.wait_for(clock, std::chrono::seconds(60));
            if(stop_flag) break;
            std::unique_lock<std::shared_mutex> lock(mutex);
            auto now = std::time(nullptr);
            for(auto it = storage.begin(); it != storage.end();) {
                if(it->second->expires_at <= now) {
                    it = storage.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void session_shutdown() {
        stop_flag = true;
        cv.notify_one();
        if(cleanup_thread.joinable()) cleanup_thread.join();
    }

    std::optional<std::string> SessionData::get(const std::string& key) {
        auto it = data.find(key);
        if(it != data.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void create_session(Request& req, Response& res, int ttl, const std::string& cookie_name, const std::unique_lock<std::shared_mutex>&, const std::string& cookie_flags) {
        std::string id = random_hex(32);
        auto sd = std::make_shared<SessionData>();
        sd->expires_at = std::time(nullptr) + ttl;
        sd->id = id;
        storage[id] = sd;
        req.session = sd;
        res.header("Set-Cookie", cookie_name + "=" + id + "; " + cookie_flags);
    }

    Middleware Session(int ttl, const std::string& cookie_name, const std::string& cookie_flags) {
        static std::once_flag init_flag;
        std::call_once(init_flag, []{ cleanup_thread = std::thread(cleanup); });
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
                    value.erase(0, value.find_first_not_of(' '));
                    value.erase(value.find_last_not_of(' ') + 1);
                    if(key == cookie_name) {
                        session_id = value;
                        if(value.size() != 64 || value.find_first_not_of("0123456789abcdef") != std::string::npos) {
                            session_id.reset();
                        }
                        break;
                    }
                }
            }

            bool found = false;
            {
                std::shared_lock<std::shared_mutex> slock(mutex);
                if(session_id) {
                    auto it = storage.find(session_id.value());
                    if(it != storage.end() && it->second->expires_at > std::time(nullptr)) {
                        it->second->expires_at.store(std::time(nullptr) + ttl);
                        req.session = it->second;
                        found = true;
                    }
                }
            }
            if(!found) {
                std::unique_lock<std::shared_mutex> ulock(mutex);
                if(session_id) {
                    auto it = storage.find(session_id.value());
                    if(it != storage.end()) storage.erase(it);
                }
                create_session(req, res, ttl, cookie_name, ulock, cookie_flags);
            }
            next();
        };
    }
}
