#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"

namespace ynet {
    Middleware session() {
        return [](Request&, Response& res, Next next) {
            next();
            std::optional<std::string> isheader = res.getHeader("Set-Cookie");
            if(isheader.has_value()) {
                res.header("Set-Cookie", isheader.value() + "; HttpOnly; Secure; SameSite=Strict");
            }
        };
    }
}
