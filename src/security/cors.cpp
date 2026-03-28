#include <algorithm>
#include "ynet/core/middleware.h"
#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include "ynet/security/cors.h"

namespace ynet {
    Middleware cors(std::vector<std::string> allowd_origins) {
        return [allowd_origins](Request& req, Response& res, Next next) {
            auto origin = req.getHeader("Origin");
            if(allowd_origins.size() == 1 && allowd_origins[0] == "*") {
                res.header("Access-Control-Allow-Origin", "*");
            } else if(origin.has_value()) {
                auto it = std::find(allowd_origins.begin(), allowd_origins.end(), origin.value());
                if(it != allowd_origins.end()) {
                    res.header("Access-Control-Allow-Origin", origin.value());
                }
            }
            next();
        };
    }
}
