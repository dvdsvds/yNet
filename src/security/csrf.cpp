#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/util/crypto.h"
#include "ynet/security/csrf.h"

namespace ynet {
    Middleware Csrf() {
        return [](Request& req, Response& res, Next next) {
            const std::string method = req.getMethod();
            if(method == "GET") {
                std::string token = random_hex(32);
                req.session.set("_csrf", token);
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
                    if(token_val.empty()) {
                        auto csrf = req.getFormParam("_csrf");
                        if(csrf.has_value()) token_val = csrf.value();
                    }
                }
                auto stored = req.session.get("_csrf");
                if(!token_val.empty() && stored.has_value() && stored.value() == token_val) {
                    next();
                } else {
                    res.status(403).body("Forbidden");
                }
            }
        };
    }
}
