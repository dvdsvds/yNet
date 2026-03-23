#include "ynet/middleware.h"
#include "ynet/request.h"
#include "ynet/response.h"
#include <ynet/security/cors.h>

namespace ynet {
    Middleware session() {
        return [](const Request& req, Response& res, Next next) {
            next();
            std::optional<std::string> isheader = res.getHeader("Set-Cookie");
            if(isheader.has_value()) {
                res.header("Set-Cookie", isheader.value() + "; HttpOnly; Secure; SameSite=Strict");
            }
        };
    }
}
