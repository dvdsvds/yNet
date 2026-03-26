#include "ynet/middleware.h"
#include "ynet/request.h"
#include "ynet/response.h"
#include "ynet/util/crypto.h"
#include <ynet/security/csrf.h>

namespace ynet {
    std::unordered_map<std::string, std::string> csrfs;
    Middleware Csrf() {
        return [](Request& req, Response& res, Next next) {
            const std::string method = req.getMethod();            
            if(method == "GET") {
                std::string token = random_hex(32);
                csrfs[token] = token;
                res.header("X-CSRF-Token", token);
                req.setCsrfToken(token);
                next();
            } else if(method == "POST" || method == "PUT" || method == "DELETE") {
                std::string token_val;
                auto token = req.getHeader("X-CSRF-Token");
                if(token.has_value()) {
                    token_val = token.value();
                } else {
                    for(const auto& part : req.getParts()) {
                        if(part.name == "_csrf") {
                            token_val = std::string(part.data, part.data_len);
                            break;
                        }
                    }
                }
                if(!token_val.empty() && csrfs.count(token_val)) {
                    next();
                } else {
                    res.status(403).body("Forbidden");
                }
            }
        };
    }
}
