# yNet

A lightweight HTTP/HTTPS framework built with C++

**[Korean](../README.md)**

---

## Introduction

yNet is a lightweight HTTP/HTTPS framework based on C++20. It provides core features for building web servers, including routing, middleware, security, and utilities.

## Tech Stack

- **Language**: C++20
- **Build**: CMake 3.20+
- **TLS**: OpenSSL
- **Platform**: POSIX (Linux)

## Features

### HTTP Server Core
Router-based request handling, middleware chain, request/response parsing, TCP listener and connection management

### Security
HTTPS (TLS), CORS, CSRF token verification, session management, rate limiter, input sanitization, automatic security headers

### Utilities
JSON parser/serializer, URL encoding/decoding/query string parsing, MIME type mapping, TCP client, request logger, SHA-256 hashing

## Project Structure

```
include/ynet/
├── config.h
├── middleware.h
├── request.h
├── response.h
├── router.h
├── server.h
├── net/
│   ├── connection.h
│   ├── tcp_client.h
│   └── tcp_listener.h
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
    ├── json.h
    ├── logger.h
    ├── mime.h
    └── url.h
```

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

## License

BSL-1.0 (Boost Software License)

