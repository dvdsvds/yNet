# yNet

C++20 기반 풀스택 HTTP/HTTPS 웹 프레임워크

**[English](docs/README_en.md)**

---

## 소개

yNet은 C++20 기반의 HTTP/HTTPS 웹 프레임워크입니다. 라우팅, 미들웨어, 보안, 파일 업로드, WebSocket, 템플릿 엔진까지 웹 서버에 필요한 기능을 제공합니다.

## 기술 스택

- **언어**: C++20
- **빌드**: CMake 3.20+
- **TLS**: OpenSSL
- **이벤트 루프**: epoll (Linux)
- **표준**: POSIX (Linux)

## 주요 기능

### HTTP 서버 코어
라우터 기반 요청 처리, 미들웨어 체인, 요청/응답 파싱, TCP 리스너 및 커넥션 관리, HTTP keep-alive

### 동시성
epoll 이벤트 루프, 스레드 풀

### WebSocket
핸드셰이크, 프레임 송수신, onOpen/onMessage/onClose 콜백

### 보안
HTTPS (TLS), CORS, CSRF 토큰 검증, 세션 관리, Rate Limiter, 입력 검증 (Sanitizer), 보안 헤더 자동 적용

### 파일 업로드
multipart/form-data 파싱, 파일 저장 (path traversal 방지), 업로드 크기 제한 (413 응답), 파일명 충돌 방지 (자동 번호 증가)

### 템플릿 엔진
변수 치환 `{{var}}`, HTML 자동 이스케이프, raw 출력 `{{{var}}}`, 조건부 `{{#if}}`, 반복 `{{#each}}`, 파일 인클루드 `{{> partial}}`, 파일 변경 감지 캐싱

### 정적 파일 서빙
디렉토리 기반 정적 파일 제공, directory traversal 방지

### 유틸리티
JSON 파서/직렬화, URL 인코딩/디코딩/쿼리스트링 파싱, MIME 타입 매핑, TCP 클라이언트, 요청 로거, SHA-256 해싱

---

## 빌드 및 실행

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
mkdir build && cd build
cmake ..
make
./example
```

### 요구사항

- C++20
- CMake 3.20+
- OpenSSL

### HTTPS 사용 시

```bash
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes
```

---

## API 레퍼런스

### Config

서버 설정 구조체입니다.

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

// 라우터 마운트
server.mount(router);

// 미들웨어 등록
server.use(ynet::Logger());

// 정적 파일 서빙
server.serveStatic("/static", "./public");

// WebSocket 라우트
server.ws("/ws", [](ynet::WebSocket& ws) { ... });

// 커스텀 에러 핸들러
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

// 서버 시작
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
// 메서드, 경로, 버전
req.getMethod();   // "GET", "POST", ...
req.getPath();     // "/upload"
req.getVersion();  // "HTTP/1.1"

// 헤더
req.getHeader("Content-Type");  // std::optional<std::string>

// 쿼리 파라미터 (?key=value)
req.getQueryParam("key");       // std::optional<std::string>

// form-urlencoded / multipart 폼 데이터
req.getFormParam("username");   // std::optional<std::string>

// multipart 파일 파트
req.getParts();                 // const std::vector<Part>&

// raw body
req.getBody();                  // const std::string&

// 클라이언트 IP
req.getClientIP();

// CSRF 토큰
req.getCsrfToken();

// 파싱 에러 체크
req.isParseError();             // bool
req.getErrorCode();             // int (400 등)
```

### Response

```cpp
// 기본 사용
res.status(200).header("Key", "Value").body("content");

// 단축 메서드
res.json("{\"status\":\"ok\"}");    // Content-Type: application/json + 200
res.html("<h1>Hello</h1>");         // Content-Type: text/html; charset=utf-8 + 200
res.redirect("/new-path");          // 302 리다이렉트
res.redirect("/new-path", 301);     // 301 리다이렉트
```

### 미들웨어

```cpp
// 내장 미들웨어
server.use(ynet::SecureHeaders());
server.use(ynet::Sanitizer());
server.use(ynet::RateLimiter(100, 60));  // 60초에 100번 제한
server.use(ynet::Logger());
server.use(ynet::cors({"*"}));
server.use(ynet::Csrf());
server.use(ynet::session());

// 커스텀 미들웨어
server.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // 요청 전 처리
    next();
    // 요청 후 처리
});
```

### 파일 업로드

```cpp
// 업로드 폼 (CSRF 토큰 포함)
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

// 업로드 처리
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
        ws.sendFrame(msg, 0x1);  // 텍스트 프레임 에코
    });
    ws.onClose([]() {
        std::cout << "disconnected" << std::endl;
    });
    ws.run();
});
```

### 템플릿 엔진

```cpp
ynet::TemplateEngine engine;

ynet::Object obj;
obj["username"] = ynet::JsonValue{std::string("dvd")};
obj["logged_in"] = ynet::JsonValue{true};

ynet::JsonValue vars{obj};
std::string html = engine.render("templates/page.html", vars);
res.html(html);
```

템플릿 문법:
- `{{var}}` — 변수 출력 (HTML 이스케이프)
- `{{{var}}}` — raw 출력 (이스케이프 없음)
- `{{#if var}}...{{/if}}` — 조건부 렌더링
- `{{#each items}}...{{/each}}` — 반복 렌더링
- `{{> partial}}` — 파일 인클루드

### FetchContent 사용법

외부 프로젝트에서 yNet을 가져다 쓸 수 있습니다.

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

또는 설치 후 `find_package`로 사용:

```cmake
find_package(ynet REQUIRED)
target_link_libraries(your_app ynet::ynet)
```

---

## 프로젝트 구조

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

## 라이선스

MIT License
