# yNet

A full-stack HTTP/HTTPS web framework built with C++20

**[한국어](../README.md)** | **[Website](https://ynetcpp.dev)**

---

## Introduction

yNet is a lightweight HTTP/HTTPS web framework based on C++20. It provides routing, middleware, security, file uploads, WebSocket, template engine, and caching — everything you need for a web server.

## Tech Stack

- **Language**: C++20
- **Build**: CMake 3.20+
- **TLS**: OpenSSL
- **Event Loop**: epoll (Linux)
- **Standard**: POSIX (Linux)

## Features

### HTTP Server Core
Router-based request handling, middleware chain, request/response parsing, TCP listener and connection management, HTTP keep-alive

### Concurrency
epoll event loop, thread pool

### WebSocket
Handshake, frame send/receive, onOpen/onMessage/onClose callbacks

### Security
HTTPS (TLS), CORS, CSRF token validation, session management (auto-creation, expiration, sliding expiration), Rate Limiter, input validation (Sanitizer), automatic security headers (custom CSP support), PathGuard (built-in WAF: malicious path detection, automatic IP blacklisting, whitelist), URL normalization (decoding, double encoding rejection, path normalization)

### Session
Cookie-based session management, automatic session ID generation (`Set-Cookie`), expiration and sliding expiration, `req.session->get()`/`->set()` for session data storage/retrieval, automatic security flags (HttpOnly, Secure, SameSite)

### Cache
File cache (mtime-based refresh), abstract `CachePolicy` class for swappable eviction policies, built-in LRU policy, configurable max cache entries

### File Upload
multipart/form-data parsing, file saving (path traversal prevention), upload size limit (413 response), filename collision prevention (auto-increment)

### Template Engine
One-line rendering with `res.render()`, variable substitution `{{var}}`, automatic HTML escaping, raw output `{{{var}}}`, conditionals `{{#if}}`, loops `{{#each}}`, file includes `{{> partial}}`, file change detection caching

### Static File Serving
Directory-based static file serving, directory traversal prevention

### Configuration
Auto-generated `.conf` file with `ConfigParser`, supports port/TLS/body size/cache settings

### Utilities
JSON parser/serializer, URL encoding/decoding/query string parsing, MIME type mapping, TCP client, request logger, SHA-256 hashing

---

## Quick Start

### CLI Installation

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
./install.sh
```

### Create a Project

```bash
ynet new myapp
cd myapp
ynet build
ynet run
```

Visit `http://localhost:8080` to see the default page.

### CLI Commands

```bash
ynet new <project-name>   # Create new project
ynet build                # Build
ynet run                  # Run
ynet clean                # Clean build files
ynet help                 # Help
```

### Generated Project Structure

```
myapp/
├── CMakeLists.txt        # Auto-downloads yNet via FetchContent
├── src/
│   └── main.cpp          # Default handler included
├── static/
└── templates/
    └── index.html        # Default Welcome page
```

### Requirements

- Linux (epoll-based)
- C++20
- CMake 3.20+
- OpenSSL

### HTTPS Setup

Set `tls=on` in the config file and place certificates in the config directory.

```bash
openssl req -x509 -newkey rsa:2048 -keyout config/key.pem -out config/cert.pem -days 365 -nodes
```

### Manual Build (without CLI)

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
mkdir build && cd build
cmake ..
make
./example
```

---

## Example

```cpp
#include <ynet/app.h>

int main() {
    ynet::App app;

    app.get("/").html("<h1>Hello yNet!</h1>");

    app.get("/api/hello").handle([](ynet::Request& req, ynet::Response& res) {
        auto name = req.getQueryParam("name");
        res.json(R"({"message": "hello, )" + name.value_or("world") + R"("})");
    });

    app.cors("*");
    app.session();
    app.listen();
}
```

---

## API Reference

### App

`App` is the entry point of yNet. It provides routing, middleware, and server configuration through a single interface.

```cpp
ynet::App app;

// Routing
app.get("/path").html("<h1>Hello</h1>");
app.get("/path").json(R"({"ok": true})");
app.get("/path").file("static/page.html");
app.get("/path").handle([](ynet::Request& req, ynet::Response& res) { ... });
app.post("/path").handle(handler);
app.put("/path").handle(handler);
app.del("/path").handle(handler);

// Middleware
app.cors("*");
app.logger();
app.csrf();
app.sanitizer();
app.secureHeaders();                            // Default CSP
app.secureHeaders("default-src 'self'; ...");   // Custom CSP
app.session();
app.rateLimit(100, 60);

// PathGuard (Built-in WAF)
app.addWhitelist("123.456.78.90");              // Register whitelist IP
app.pathGuard();                                 // Activate PathGuard

// Custom Middleware
app.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-request processing
    next();
});

// Static Files
app.serveStatic("/static", "./public");

// WebSocket
app.ws("/ws", wsHandler);

// Error Handlers
app.onError(404).html("<h1>404 Not Found</h1>");
app.onError(500).html("<h1>500 Internal Server Error</h1>");

// Start Server
app.listen();
```

### Config File

A `config/project-name.conf` file is auto-generated on first run.

```
port=8080
bind=0.0.0.0
tls=on
cert=cert.pem
key=key.pem
max_body=1MB
max_upload=10MB
max_header=8KB
max_headers=64
header_timeout=5000
body_timeout=10000
max_connections=1024
max_cache=1024
```

### Request

```cpp
// Method, path, version
req.getMethod();   // "GET", "POST", ...
req.getPath();     // "/upload"
req.getVersion();  // "HTTP/1.1"

// Headers
req.getHeader("Content-Type");  // std::optional<std::string>

// Query parameters (?key=value)
req.getQueryParam("key");       // std::optional<std::string>

// form-urlencoded / multipart form data
req.getFormParam("username");   // std::optional<std::string>

// Multipart file parts
req.getParts();                 // const std::vector<Part>&

// Raw body
req.getBody();                  // const std::string&

// Client IP
req.getClientIP();

// CSRF token
req.getCsrfToken();

// Session data
req.session->set("key", "value");
req.session->get("key");        // std::optional<std::string>

// Parse error check
req.isParseError();             // bool
req.getErrorCode();             // int (400, etc.)
```

### Response

```cpp
// Basic usage
res.status(200).header("Key", "Value").body("content");

// Shortcut methods
res.json("{\"status\":\"ok\"}");    // Content-Type: application/json + 200
res.html("<h1>Hello</h1>");         // Content-Type: text/html; charset=utf-8 + 200
res.redirect("/new-path");          // 302 redirect
res.redirect("/new-path", 301);     // 301 redirect

// Template rendering
res.render("templates/page.html", vars);  // Render with template engine + text/html response
```

### Session

Register the session middleware for automatic session management.

```cpp
app.session();

app.get("/profile").handle([](ynet::Request& req, ynet::Response& res) {
    // Store data in session
    req.session->set("user_id", "123");

    // Retrieve data from session
    auto user = req.session->get("user_id");
    if(user.has_value()) {
        res.json(R"({"user": ")" + user.value() + R"("})");
    } else {
        res.status(401).json(R"({"error": "not logged in"})");
    }
});
```

How it works:
- Auto-generates session ID + `Set-Cookie` on first request
- Restores session via Cookie session ID on subsequent requests
- Automatic security flags: `HttpOnly`, `Secure`, `SameSite=Strict`
- Default TTL 1 hour, renewed on each request (sliding expiration)
- Invalid or expired sessions are automatically replaced with new ones
- Background thread periodically cleans up expired sessions
- Session ID format validation (64-char hex)
- shared_mutex-based read/write concurrency optimization

### Cache

Built-in LRU cache with swappable policies.

```cpp
// Default (LRU)
ynet::Cache cache(1024);

// Custom policy
class MyPolicy : public ynet::CachePolicy {
public:
    void evict(std::unordered_map<std::string, ynet::CacheEntry>& entries) override {
        // Custom eviction logic
    }
};

ynet::Cache cache(1024, std::make_unique<MyPolicy>());
```

Set max cache entries in the config file:
```
max_cache=2048
```

### Middleware

```cpp
// Built-in middleware
app.cors("*");                                  // CORS
app.logger();                                   // Request logger
app.csrf();                                     // CSRF token validation
app.sanitizer();                                // Input validation
app.secureHeaders();                            // Security headers (default CSP)
app.secureHeaders("default-src 'self'; ...");   // Custom CSP
app.session();                                  // Session management
app.rateLimit(100, 60);                         // 100 requests per 60 seconds

// PathGuard (Built-in WAF)
app.addWhitelist("123.456.78.90");              // Register whitelist IP
app.pathGuard();                                 // Activate PathGuard

// Custom middleware
app.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-request processing
    next();
    // Post-request processing
});
```

### PathGuard (Built-in WAF)

PathGuard is a built-in WAF for the framework. It automatically detects known malicious path scans and blocks the offending IPs.

```cpp
// Basic usage (automatic protection with built-in patterns)
app.addWhitelist("123.456.78.90");  // Whitelist admin IP
app.pathGuard();                     // Activate
app.listen();
```

Default blocked patterns:

| Type | Patterns |
|------|----------|
| Exact match | `/wp-login.php`, `/phpMyAdmin`, `/pma`, `/backup.sql`, `/db.zip`, `/swagger-ui.html`, `/v2/api-docs`, `/docker-compose.yml`, `/Dockerfile`, `/package.json`, `/.well-known/security.txt` |
| Prefix match | `/.env`, `/.git`, `/.svn`, `/.aws`, `/.ssh`, `/.vscode`, `/wp-admin`, `/cgi-bin`, `/actuator`, `/node_modules` |
| Extension match | `.php`, `.sql`, `.bak`, `.old`, `.zip` |

Custom pattern addition:

```cpp
ynet::PathGuard pg;
pg.addExact("/secret-admin");       // Add exact match
pg.addPrefix("/.docker");           // Add prefix match
pg.addExt(".conf");                 // Add extension match
app.use(pg.toMiddleware());
```

How it works:
- Whitelisted IPs always pass through
- A single pattern match immediately blacklists the IP
- Blacklisted IPs are blocked on all subsequent requests (including legitimate paths)
- Checks run after URL decoding + path normalization, making encoding bypasses impossible
- Double-encoded requests are rejected with 400 at the parsing stage
- Thread-safe blacklist via mutex

### URL Normalization

All request paths are automatically normalized at the parsing stage.

- URL decoding: `/%77p-admin` → `/wp-admin`
- Double encoding rejection: `/%2577p-admin` → 400 Bad Request
- Null byte rejection: `/safe%00/../.env` → 400 Bad Request
- Double slash removal: `//wp-admin` → `/wp-admin`
- Path traversal removal: `/foo/../.env` → `/.env`
- Trailing slash removal: `/wp-admin/` → `/wp-admin`

### File Upload

```cpp
app.get("/upload").handle([](ynet::Request& req, ynet::Response& res) {
    std::string token = req.getCsrfToken();
    std::string html = R"(
        <form action="/upload" method="POST" enctype="multipart/form-data">
            <input type="file" name="file">
            <input type="hidden" name="_csrf" value=")" + token + R"(">
            <button type="submit">Upload</button>
        </form>
    )";
    res.html(html);
});

app.post("/upload").handle([](ynet::Request& req, ynet::Response& res) {
    for(const auto& part : req.getParts()) {
        if(!part.filename.empty()) {
            ynet::saveFile(part, "./uploads");
        }
    }
    res.status(200).body("upload success");
});
```

### WebSocket

```cpp
app.ws("/ws", [](ynet::WebSocket& ws) {
    ws.onOpen([]() {
        std::cout << "connected" << std::endl;
    });
    ws.onMessage([&ws](const std::string& msg) {
        ws.sendFrame("echo: " + msg, 0x1);  // Echo text frame
    });
    ws.onClose([]() {
        std::cout << "disconnected" << std::endl;
    });
    ws.run();
});
```

### Template Engine

```cpp
app.get("/").handle([](ynet::Request& req, ynet::Response& res) {
    ynet::Object vars;
    vars["username"] = ynet::JsonValue{std::string("dvd")};
    vars["logged_in"] = ynet::JsonValue{true};
    res.render("templates/page.html", ynet::JsonValue{vars});
});
```

Template syntax:
- `{{var}}` — Variable output (HTML escaped)
- `{{{var}}}` — Raw output (no escaping)
- `{{#if var}}...{{/if}}` — Conditional rendering
- `{{#each items}}...{{/each}}` — Loop rendering
- `{{> partial}}` — File include

### FetchContent Usage

Use yNet in external projects.

```cmake
include(FetchContent)
FetchContent_Declare(
    ynet
    GIT_REPOSITORY https://github.com/dvdsvds/yNet.git
    GIT_TAG main
)
FetchContent_MakeAvailable(ynet)

target_link_libraries(your_app ynet)
```

Or install and use with `find_package`:

```cmake
find_package(ynet REQUIRED)
target_link_libraries(your_app ynet::ynet)
```

---

## Project Structure

```
include/ynet/
├── app.h
├── cache/
│   ├── cache.h
│   ├── cache_entry.h
│   ├── cache_policy.h
│   └── lru_policy.h
├── core/
│   ├── config.h
│   ├── middleware.h
│   ├── request.h
│   ├── response.h
│   ├── router.h
│   ├── server.h
│   └── static_file.h
├── net/
│   ├── connection.h
│   ├── event_loop.h
│   ├── tcp_client.h
│   ├── tcp_listener.h
│   ├── thread_pool.h
│   └── websocket.h
├── security/
│   ├── cors.h
│   ├── csrf.h
│   ├── path_guard.h
│   ├── rate_limiter.h
│   ├── sanitizer.h
│   ├── secure_headers.h
│   ├── session.h
│   ├── session_data.h
│   └── tls_context.h
└── util/
    ├── config_parser.h
    ├── crypto.h
    ├── file_saver.h
    ├── json.h
    ├── logger.h
    ├── mime.h
    ├── multipart_parser.h
    ├── template_engine.h
    └── url.h

src/
├── app.cpp
├── cache/
│   ├── cache.cpp
│   └── lru_policy.cpp
├── core/
│   ├── request.cpp
│   ├── response.cpp
│   ├── router.cpp
│   ├── server.cpp
│   └── static_file.cpp
├── net/
│   ├── connection.cpp
│   ├── event_loop.cpp
│   ├── tcp_client.cpp
│   ├── tcp_listener.cpp
│   ├── thread_pool.cpp
│   └── websocket.cpp
├── security/
│   ├── cors.cpp
│   ├── csrf.cpp
│   ├── path_guard.cpp
│   ├── rate_limiter.cpp
│   ├── sanitizer.cpp
│   ├── secure_headers.cpp
│   ├── session.cpp
│   └── tls_context.cpp
└── util/
    ├── config_parser.cpp
    ├── crypto.cpp
    ├── file_saver.cpp
    ├── json.cpp
    ├── logger.cpp
    ├── mime.cpp
    ├── multipart_parser.cpp
    ├── template_engine.cpp
    └── url.cpp
```

## License

Apache License 2.0
