#include <ynet/app.h>
#include "ynet/core/server.h"
#include "ynet/util/logger.h"
#include "ynet/security/rate_limiter.h"
#include "ynet/security/csrf.h"
#include "ynet/security/cors.h"
#include "ynet/security/sanitizer.h"
#include "ynet/security/secure_headers.h"
#include "ynet/security/session.h"
#include "ynet/security/path_guard.h"

using namespace ynet;

ErrorRoute App::onError(int code) { return ErrorRoute(server, code); }
void App::cors(const std::string& origin) { server.use(ynet::cors({origin})); }
void App::rateLimit(int max_requests, int period_seconds) { server.use(ynet::RateLimiter(max_requests, period_seconds)); };
void App::logger() { server.use(ynet::Logger()); }
void App::csrf() { server.use(ynet::Csrf()); }
void App::sanitizer() { server.use(ynet::Sanitizer()); }
void App::secureHeaders(const std::string& csp) { server.use(ynet::SecureHeaders(csp)); }
void App::session() { server.use(ynet::Session()); }
void App::serveStatic(const std::string& prefix, const std::string& dir) { server.serveStatic(prefix, dir); }
void App::ws(const std::string& path, WsHandler handler) { server.ws(path, handler); }

void App::listen() {
    server.mount(router);
    server.start();
}

void App::pathGuard() {
    server.use(path_guard.toMiddleware());
}
