# yNet

A C++20 full-stack HTTP/HTTPS web framework

**[한국어](../README.md)** | **[Website](https://ynetcpp.dev)**

---

## Introduction

yNet is a C++20-based HTTP/HTTPS web framework. It provides everything a web server needs: routing, middleware, security, file uploads, WebSocket, template engine, and caching.

## Tech Stack

- **Language**: C++20
- **Build**: CMake 3.20+
- **TLS**: OpenSSL
- **Event Loop**: epoll (Linux)
- **Standard**: POSIX (Linux)

## Features

### HTTP Server Core
- Router-based request handling
- Middleware chain
- Request/response parsing
- TCP listener and connection management
- HTTP keep-alive

### Concurrency
- epoll event loop
- Thread pool

### WebSocket
- Handshake
- Frame send/receive
- onOpen / onMessage / onClose callbacks

### Security
- HTTPS (TLS)
- CORS
- CSRF token validation
- Session management (auto-generation, expiration, sliding expiration)
- Rate limiter
- Input validation (Sanitizer)
- Automatic security headers (custom CSP supported)
- PathGuard (built-in WAF: malicious path detection, IP auto-blacklisting, whitelisting)
- URL normalization (decoding, double-encoding rejection, path canonicalization)

### Session
- Cookie-based session management
- Automatic session ID generation (`Set-Cookie`)
- Expiration and sliding expiration
- Store/retrieve session data with `req.session->get()` / `->set()`
- Security flags applied automatically (HttpOnly, Secure, SameSite)

### Cache
- File cache (mtime-based invalidation)
- Abstract `CachePolicy` class for swappable eviction policies
- Built-in LRU policy
- Configurable max cache size via config

### File Upload
- multipart/form-data parsing
- File storage (path traversal protection)
- Upload size limit (413 response)
- Filename collision avoidance (auto-incrementing suffix)

### Template Engine
- Single-line rendering with `res.render()`
- Variable substitution `{{var}}`
- Automatic HTML escaping
- Raw output `{{{var}}}`
- Conditional `{{#if}}`
- Loop `{{#each}}`
- File include `{{> partial}}`
- File-change detection caching

### Static File Serving
- Directory-based static file delivery
- Directory traversal protection

### Configuration
- Auto-generated and parsed `.conf` files (`ConfigParser`)
- Port, TLS, body size, cache, and more

### Utilities
- JSON parser/serializer
- URL encoding/decoding/querystring parsing
- MIME type mapping
- TCP client
- Request logger
- SHA-256 hashing

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

Visit `http://localhost:8080` to see the default welcome page.

### CLI Commands

```bash
ynet new <project-name>   # Create a new project
ynet build                # Build
ynet run                  # Run
ynet clean                # Remove build files
ynet help                 # Show help
```

### Generated Project Structure

```
myapp/
├── CMakeLists.txt        # Auto-downloads yNet via FetchContent
├── src/
│   └── main.cpp          # Includes a default handler
├── static/
└── templates/
    └── index.html        # Default welcome page
```

### Requirements

- Linux (epoll-based)
- C++20
- CMake 3.20+
- OpenSSL

### Using HTTPS

Set `tls=on` in the config file and place your certificate in the config directory.

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

`App` is the entry point of yNet. It exposes routing, middleware, and server configuration through a single interface.

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

// PathGuard (built-in WAF)
app.addWhitelist("123.456.78.90");              // Register whitelist IP
app.pathGuard();                                 // Enable PathGuard

// Custom middleware
app.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-request logic
    next();
});

// Static files
app.serveStatic("/static", "./public");

// WebSocket
app.ws("/ws", wsHandler);

// Error handlers
app.onError(404).html("<h1>404 Not Found</h1>");
app.onError(500).html("<h1>500 Internal Server Error</h1>");

// Start the server
app.listen();
```

### Config File

On the first run, `config/<project-name>.conf` is automatically generated.

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

// multipart file parts
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

// Parse error checks
req.isParseError();             // bool
req.getErrorCode();             // int (e.g. 400)
```

### Response

```cpp
// Basic usage
res.status(200).header("Key", "Value").body("content");

// Shorthand methods
res.json("{\"status\":\"ok\"}");    // Content-Type: application/json + 200
res.html("<h1>Hello</h1>");         // Content-Type: text/html; charset=utf-8 + 200
res.redirect("/new-path");          // 302 redirect
res.redirect("/new-path", 301);     // 301 redirect

// Template rendering
res.render("templates/page.html", vars);  // Renders via template engine + text/html response
```

### Session

Registering the session middleware enables automatic session management.

```cpp
app.session();

app.get("/profile").handle([](ynet::Request& req, ynet::Response& res) {
    // Store data in the session
    req.session->set("user_id", "123");

    // Retrieve data from the session
    auto user = req.session->get("user_id");
    if(user.has_value()) {
        res.json(R"({"user": ")" + user.value() + R"("})");
    } else {
        res.status(401).json(R"({"error": "not logged in"})");
    }
});
```

How it works:
- Generates a session ID on the first request and sends `Set-Cookie`
- Restores the session from the cookie ID on subsequent requests
- Security flags applied automatically: `HttpOnly`, `Secure`, `SameSite=Strict`
- Default TTL: 1 hour, refreshed on every request (sliding expiration)
- Invalid or expired sessions are automatically replaced with a new one
- A background thread periodically cleans up expired sessions
- Session ID format is validated (64-char hex)
- Read/write concurrency optimized via `shared_mutex`

### Cache

An LRU cache is applied by default. You can swap the policy.

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

The max cache size can also be set in the config file:
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

// PathGuard (built-in WAF)
app.addWhitelist("123.456.78.90");              // Register whitelist IP
app.pathGuard();                                 // Enable PathGuard

// Custom middleware
app.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-request logic
    next();
    // Post-request logic
});
```

### PathGuard (Built-in WAF)

PathGuard is yNet's built-in WAF. It automatically detects known malicious path scans and blocks the offending IP.

```cpp
// Basic usage (auto-protection with built-in patterns)
app.addWhitelist("123.456.78.90");  // Whitelist an admin IP
app.pathGuard();                     // Enable
app.listen();
```

## Block Patterns

Paths commonly probed by malicious scanners and automated bots are blocked via three matching strategies.

### Exact Match

Blocks requests whose path **exactly equals** one of the following.

| Pattern | Description |
|---------|-------------|
| `/wp-login.php` | WordPress login page scan |
| `/phpMyAdmin` | phpMyAdmin admin console probe |
| `/pma` | phpMyAdmin shorthand probe |
| `/backup.sql` | Exposed DB backup file probe |
| `/db.zip` | Compressed DB dump probe |
| `/swagger-ui.html` | Swagger UI exposure check |
| `/v2/api-docs` | Swagger/OpenAPI spec exposure check |
| `/docker-compose.yml` | Docker Compose config exposure (credential leak risk) |
| `/Dockerfile` | Dockerfile exposure (internal structure leak) |
| `/package.json` | Node.js dependency info exposure |
| `/.well-known/security.txt` | Automated security contact lookup |

### Prefix Match

Blocks requests whose path **starts with** one of the following, including all sub-paths (e.g. `/.git/config`, `/wp-admin/install.php`).

| Pattern | Description |
|---------|-------------|
| `/.env` | Environment file (API key / DB password leak risk) |
| `/.git` | Git repo exposure (full source code leak risk) |
| `/.svn` | SVN repo exposure |
| `/.aws` | AWS credentials directory |
| `/.ssh` | SSH keys directory |
| `/.vscode` | VSCode settings (internal path / tool info leak) |
| `/wp-admin` | WordPress admin page scan |
| `/cgi-bin` | Legacy CGI script scan |
| `/actuator` | Spring Boot Actuator endpoint (env / metrics exposure) |
| `/node_modules` | Direct access to Node.js dependency directory |

### Extension Match

Blocks requests whose path **ends with** one of the following.

| Pattern | Description |
|---------|-------------|
| `.php` | PHP script scan (this service does not use PHP) |
| `.sql` | SQL dump file probe |
| `.bak` | Backup files (`config.php.bak`, etc.) probe |
| `.old` | Legacy-version file probe |
| `.zip` | Compressed source / backup file probe |


Adding custom patterns:
```cpp
ynet::PathGuard pg;
pg.addExact("/secret-admin");       // Add exact match
pg.addPrefix("/.docker");           // Add prefix match
pg.addExt(".conf");                 // Add extension match
app.use(pg.toMiddleware());
```

How it works:
- Whitelisted IPs always pass through
- A single pattern match immediately blacklists the source IP
- Blacklisted IPs are blocked on all subsequent requests (including legitimate paths)
- Inspection runs after URL decoding and path normalization, so encoding-based bypasses fail
- Double-encoded requests are rejected with 400 at the parsing stage
- Thread-safe blacklist backed by a mutex

### URL Normalization

Every request path is automatically normalized at the parsing stage.

- URL decoding: `/%77p-admin` → `/wp-admin`
- Double-encoding rejection: `/%2577p-admin` → 400 Bad Request
- Null-byte rejection: `/safe%00/../.env` → 400 Bad Request
- Double-slash collapsing: `//wp-admin` → `/wp-admin`
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
        ws.sendFrame("echo: " + msg, 0x1);  // Echo a text frame
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
- `{{var}}` — variable output (HTML-escaped)
- `{{{var}}}` — raw output (no escaping)
- `{{#if var}}...{{/if}}` — conditional rendering
- `{{#each items}}...{{/each}}` — loop rendering
- `{{> partial}}` — file include

### Using with FetchContent

You can pull yNet into external projects.

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

Or install it and use `find_package`:

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
    ├── json.cpd
    ├── logger.cpp
    ├── mime.cpp
    ├── multipart_parser.cpp
    ├── template_engine.cpp
    └── url.cpp
```

## License
Apache License 2.0
