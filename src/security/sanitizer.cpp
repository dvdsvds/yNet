#include "ynet/middleware.h"
#include "ynet/request.h"
#include "ynet/response.h"
#include <ynet/security/sanitizer.h>

namespace ynet {
    Middleware Sanitizer() {
        return [](const Request& req, Response& res, Next next) {
            std::string path = req.getPath();
            std::vector<std::string> blocked = {"..", "%2e%2e", "%00"};
            for(auto& pattern : blocked) {
                if(path.find(pattern) != std::string::npos) {
                    res.status(400).body("Bad Request");
                    return;
                }
            }
            next();
        };
    }
}
