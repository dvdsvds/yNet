# yNet

A full-stack HTTP/HTTPS web framework built with C++20

**[한국어](../README.md)**

---

## Introduction

yNet is an HTTP/HTTPS web framework built with C++20. It provides routing, middleware, security, file uploads, WebSocket, template engine, and caching — everything you need for a web server.

## Tech Stack

- **Language**: C++20
- **Build**: CMake 3.20+
- **TLS**: OpenSSL
- **Event Loop**: epoll (Linux)
- **Standard**: POSIX (Linux)

## Key Features

### HTTP Server Core
Router-based request handling, middleware chain, request/response parsing, TCP listener and connection management, HTTP keep-alive

### Concurrency
epoll event loop, thread pool

### WebSocket
Handshake, frame send/receive, onOpen/onMessage/onClose callbacks

### Security
HTTPS (TLS), CORS, CSRF token validation, session management (auto-generation, expiration, sliding expiration), rate limiter, input sanitizer, automatic security headers

### Session
Cookie-based session management, automatic session ID generation (`Set-Cookie`), expiration with sliding expiration, `req.session.get()`/`set()` for session data storage/retrieval, automatic security flags (HttpOnly, Secure, SameSite)

### Cache
File cache (mtime-based refresh), swappable eviction policy via abstract `CachePolicy` class, built-in LRU policy, configurable max cache size

### File Upload
multipart/form-data parsing, file saving (path traversal prevention), upload size limit (413 response), filename collision prevention (auto-incrementing)

### Template Engine
Variable substitution `{{var}}`, automatic HTML escaping, raw output `{{{var}}}`, conditionals `{{#if}}`, loops `{{#each}}`, file includes `{{> partial}}`, file change detection caching

### Static File Serving
Directory-based static file serving, directory traversal prevention

### Configuration
Auto-generated `.conf` file with `ConfigParser`, supports port/TLS/body size/cache settings

### Utilities
JSON parser/serializer, URL encoding/decoding/query string parsing, MIME type mapping, TCP client, request logger, SHA-256 hashing

---

## Quick Start

### Install CLI

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
ynet new <project-name>   # Create a new project
ynet build                # Build
ynet run                  # Run
ynet clean                # Clean build files
ynet help                 # Show help
```

### Generated Project Structure

```
myapp/
├── CMakeLists.txt        # Auto-downloads yNet via FetchContent
├── src/
│   └── main.cpp          # Default handlers included
├── static/
└── templates/
```

### Requirements

- Linux (epoll-based)
- C++20
- CMake 3.20+
- OpenSSL

### For HTTPS

Set `tls=on` in the config file, then generate certificates:

```bash
cd build/config
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes
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

void hello(ynet::Request& req, ynet::Response& res) {
    auto name = req.getParam("name");
    res.json(R"({"message": "hello, )" + name.value_or("world") + R"("})");
}

int main() {
    ynet::App app;
    app.get("/").html("<h1>Hello yNet!</h1>");
    app.get("/hello/:name").handle(hello);
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
app.secureHeaders();
app.session();
app.rateLimit(100, 60);

// Static files
app.serveStatic("/static", "./public");

// WebSocket
app.ws("/ws", wsHandler);

// Error handlers
app.onError(404).html("<h1>404 Not Found</h1>");
app.onError(500).html("<h1>500 Internal Server Error</h1>");

// Start server
app.listen();
```

### Config File

A config file is auto-generated at `config/project_name.conf` on first run.

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

// URL parameters (/user/:id)
req.getParam("id");             // std::optional<std::string>

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
req.session.set("key", "value");
req.session.get("key");         // std::optional<std::string>

// Parse error check
req.isParseError();             // bool
req.getErrorCode();             // int (400, etc.)
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
```

### Session

Register the session middleware for automatic session management.

```cpp
app.session();

app.get("/profile").handle([](ynet::Request& req, ynet::Response& res) {
    // Store data in session
    req.session.set("user_id", "123");

    // Retrieve data from session
    auto user = req.session.get("user_id");
    if(user.has_value()) {
        res.json(R"({"user": ")" + user.value() + R"("})");
    } else {
        res.status(401).json(R"({"error": "not logged in"})");
    }
});
```

How it works:
- Automatically generates a session ID on first request + `Set-Cookie` response
- Restores session from Cookie's session ID on subsequent requests
- Automatic security flags: `HttpOnly`, `Secure`, `SameSite=Strict`
- Default TTL of 1 hour, refreshed on each request (sliding expiration)
- Invalid or expired sessions are automatically replaced with new ones

### Cache

Built-in LRU cache is applied by default. The eviction policy is swappable.

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

Set the max cache size in the config file:
```
max_cache=2048
```

### Middleware

```cpp
// Built-in middleware
app.cors("*");                  // CORS
app.logger();                   // Request logger
app.csrf();                     // CSRF token validation
app.sanitizer();                // Input sanitizer
app.secureHeaders();            // Security headers
app.session();                  // Session management
app.rateLimit(100, 60);         // 100 requests per 60 seconds

// Custom middleware (when using Server directly)
server.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-processing
    next();
    // Post-processing
});
```

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
ynet::TemplateEngine engine;

ynet::Object obj;
obj["username"] = ynet::JsonValue{std::string("dvd")};
obj["logged_in"] = ynet::JsonValue{true};

ynet::JsonValue vars{obj};
std::string html = engine.render("templates/page.html", vars);
res.html(html);
```

Template syntax:
- `{{var}}` — Variable output (HTML escaped)
- `{{{var}}}` — Raw output (no escaping)
- `{{#if var}}...{{/if}}` — Conditional rendering
- `{{#each items}}...{{/each}}` — Loop rendering
- `{{> partial}}` — File include

### FetchContent

You can use yNet in external projects via CMake FetchContent.

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

MIT License
