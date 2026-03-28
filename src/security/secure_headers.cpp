#include "ynet/core/middleware.h"
#include "ynet/core/response.h"
#include "ynet/core/request.h"
#include "ynet/security/secure_headers.h"

namespace ynet {
    Middleware SecureHeaders() {
        return [](Request&, Response& res, Next next) {
            res.header("X-Content-Type-Options", "nosniff");
            res.header("X-Frame-Options", "DENY");
            res.header("Referrer-Policy", "strict-origin-when-cross-origin");
            res.header("Cache-Control", "no-store");
            res.header("Content-Security-Policy", "default-src 'self'");
            next();
        };
    }
}
