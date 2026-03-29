#include <algorithm>
#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/security/cors.h"

namespace ynet {
    Middleware cors(std::vector<std::string> allowed_origins) {
        return [allowed_origins](Request& req, Response& res, Next next) {
            auto origin = req.getHeader("Origin");
            if(allowed_origins.size() == 1 && allowed_origins[0] == "*") {
                res.header("Access-Control-Allow-Origin", "*");
            } else if(origin.has_value()) {
                auto it = std::find(allowed_origins.begin(), allowed_origins.end(), origin.value());
                if(it != allowed_origins.end()) {
                    res.header("Access-Control-Allow-Origin", origin.value());
                }
            }

            if(req.getMethod() == "OPTIONS") {
                res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                res.header("Access-Control-Allow-Headers", "Content-Type, X-CSRF-Token");
                res.status(204).body("");
                return;
            }

            next();
        };
    }
}
