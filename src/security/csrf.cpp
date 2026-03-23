#include "ynet/middleware.h"
#include "ynet/request.h"
#include "ynet/response.h"
#include "ynet/util/crypto.h"
#include <ynet/security/csrf.h>

namespace ynet {
    std::unordered_map<std::string, std::string> csrfs;
    Middleware Csrf() {
        return [](const Request& req, Response& res, Next next) {
            const std::string method = req.getMethod();            
            if(method == "GET") {
                std::string token = random_hex(32);
                csrfs[token] = token;
                res.header("X-CSRF-Token", token);
                next();
            } else if(method == "POST" || method == "PUT" || method == "DELETE") {
                auto token = req.getHeader("X-CSRF-Token");
                if(token.has_value() && csrfs.count(token.value())) {
                    next();
                } else {
                    res.status(403).body("Forbidden");
                }
            }
        };
    }
}
