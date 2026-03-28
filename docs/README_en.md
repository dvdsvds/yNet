# yNet

A full-stack HTTP/HTTPS web framework built with C++20

**[한국어](../README.md)**

---

## Introduction

yNet is a C++20-based HTTP/HTTPS web framework. It provides routing, middleware, security, file uploads, WebSocket, and a template engine — everything you need for a web server.

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
HTTPS (TLS), CORS, CSRF token validation, session management, rate limiter, input sanitizer, automatic secure headers

### File Upload
multipart/form-data parsing, file saving (path traversal prevention), upload size limit (413 response), filename collision prevention (auto-incrementing numbers)

### Template Engine
Variable substitution `{{var}}`, HTML auto-escaping, raw output `{{{var}}}`, conditionals `{{#if}}`, loops `{{#each}}`, file include `{{> partial}}`, file change detection caching

### Static File Serving
Directory-based static file serving, directory traversal prevention

### Utilities
JSON parser/serializer, URL encoding/decoding/query string parsing, MIME type mapping, TCP client, request logger, SHA-256 hashing

---

## Build & Run

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
mkdir build && cd build
cmake ..
make
./example
```

### Requirements

- C++20
- CMake 3.20+
- OpenSSL

### For HTTPS

```bash
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes
```

---

## API Reference

### Config

Server configuration struct.

```cpp
ynet::Config config;
config.port = 8080;
config.use_tls = true;
config.cert_path = "cert.pem";
config.key_path = "key.pem";
config.max_upload_size = 10 * 1024 * 1024; // 10MB
```

### Server

```cpp
ynet::Server server(config);

// Mount router
server.mount(router);

// Register middleware
server.use(ynet::Logger());

// Static file serving
server.serveStatic("/static", "./public");

// WebSocket route
server.ws("/ws", [](ynet::WebSocket& ws) { ... });

// Custom error handlers
server.onError(404, [](const ynet::Request& req) {
    ynet::Response res;
    res.status(404).html("<h1>404 Not Found</h1>");
    return res;
});

server.onError(500, [](const ynet::Request& req) {
    ynet::Response res;
    res.status(500).html("<h1>500 Internal Server Error</h1>");
    return res;
});

// Start server
server.start();
```

### Router

```cpp
ynet::Router router;

router.get("/path", [](ynet::Request& req, ynet::Response& res) {
    res.status(200).body("Hello");
});

router.post("/path", [](ynet::Request& req, ynet::Response& res) {
    res.status(200).body("Created");
});
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
```

### Middleware

```cpp
// Built-in middleware
server.use(ynet::SecureHeaders());
server.use(ynet::Sanitizer());
server.use(ynet::RateLimiter(100, 60));  // 100 requests per 60 seconds
server.use(ynet::Logger());
server.use(ynet::cors({"*"}));
server.use(ynet::Csrf());
server.use(ynet::session());

// Custom middleware
server.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // Pre-processing
    next();
    // Post-processing
});
```

### File Upload

```cpp
// Upload form (with CSRF token)
router.get("/upload", [](ynet::Request& req, ynet::Response& res) {
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

// Upload handler
router.post("/upload", [](ynet::Request& req, ynet::Response& res) {
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
server.ws("/ws", [](ynet::WebSocket& ws) {
    ws.onOpen([]() {
        std::cout << "connected" << std::endl;
    });
    ws.onMessage([&ws](const std::string& msg) {
        ws.sendFrame(msg, 0x1);  // Echo text frame
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

### Using with FetchContent

You can use yNet in your project via FetchContent.

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
├── config.h
├── middleware.h
├── request.h
├── response.h
├── router.h
├── server.h
├── static_file.h
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
│   └── tls_context.h
└── util/
    ├── crypto.h
    ├── file_saver.h
    ├── json.h
    ├── logger.h
    ├── mime.h
    ├── multipart_parser.h
    ├── template_engine.h
    └── url.h
```

## License

MIT License
