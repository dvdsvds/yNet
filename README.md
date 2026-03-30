# yNet

C++20 기반 풀스택 HTTP/HTTPS 웹 프레임워크

**[English](docs/README_en.md)**

---

## 소개

yNet은 C++20 기반의 HTTP/HTTPS 웹 프레임워크입니다. 라우팅, 미들웨어, 보안, 파일 업로드, WebSocket, 템플릿 엔진, 캐시까지 웹 서버에 필요한 기능을 제공합니다.

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
HTTPS (TLS), CORS, CSRF 토큰 검증, 세션 관리 (자동 생성, 만료, sliding expiration), Rate Limiter, 입력 검증 (Sanitizer), 보안 헤더 자동 적용

### 세션
Cookie 기반 세션 관리, 자동 세션 ID 생성 (`Set-Cookie`), 만료 및 sliding expiration, `req.session.get()`/`set()`으로 세션 데이터 저장/조회, 보안 플래그 자동 적용 (HttpOnly, Secure, SameSite)

### 캐시
파일 캐시 (mtime 기반 갱신), 추상 `CachePolicy` 클래스로 eviction 정책 교체 가능, 기본 LRU 정책 제공, config에서 최대 캐시 수 설정

### 파일 업로드
multipart/form-data 파싱, 파일 저장 (path traversal 방지), 업로드 크기 제한 (413 응답), 파일명 충돌 방지 (자동 번호 증가)

### 템플릿 엔진
변수 치환 `{{var}}`, HTML 자동 이스케이프, raw 출력 `{{{var}}}`, 조건부 `{{#if}}`, 반복 `{{#each}}`, 파일 인클루드 `{{> partial}}`, 파일 변경 감지 캐싱

### 정적 파일 서빙
디렉토리 기반 정적 파일 제공, directory traversal 방지

### 설정
`.conf` 파일 자동 생성 및 파싱 (`ConfigParser`), 포트/TLS/바디 크기/캐시 등 설정 지원

### 유틸리티
JSON 파서/직렬화, URL 인코딩/디코딩/쿼리스트링 파싱, MIME 타입 매핑, TCP 클라이언트, 요청 로거, SHA-256 해싱

---

## 빠른 시작

### CLI 설치

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
./install.sh
```

### 프로젝트 생성

```bash
ynet new myapp
cd myapp
ynet build
ynet run
```

`http://localhost:8080`에 접속하면 기본 페이지가 표시됩니다.

### CLI 명령어

```bash
ynet new <project-name>   # 새 프로젝트 생성
ynet build                # 빌드
ynet run                  # 실행
ynet clean                # 빌드 파일 삭제
ynet help                 # 도움말
```

### 생성되는 프로젝트 구조

```
myapp/
├── CMakeLists.txt        # yNet을 FetchContent로 자동 다운로드
├── src/
│   └── main.cpp          # 기본 핸들러 포함
├── static/
└── templates/
```

### 요구사항

- Linux (epoll 기반)
- C++20
- CMake 3.20+
- OpenSSL

### HTTPS 사용 시

config 파일에서 `tls=on`으로 설정 후, 인증서를 생성합니다.

```bash
cd build/config
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes
```

### 수동 빌드 (CLI 없이)

```bash
git clone https://github.com/dvdsvds/yNet.git
cd yNet
mkdir build && cd build
cmake ..
make
./example
```

---

## 예제

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

## API 레퍼런스

### App

`App`은 yNet의 진입점입니다. 라우팅, 미들웨어, 서버 설정을 하나의 인터페이스로 제공합니다.

```cpp
ynet::App app;

// 라우팅
app.get("/path").html("<h1>Hello</h1>");
app.get("/path").json(R"({"ok": true})");
app.get("/path").file("static/page.html");
app.get("/path").handle([](ynet::Request& req, ynet::Response& res) { ... });
app.post("/path").handle(handler);
app.put("/path").handle(handler);
app.del("/path").handle(handler);

// 미들웨어
app.cors("*");
app.logger();
app.csrf();
app.sanitizer();
app.secureHeaders();
app.session();
app.rateLimit(100, 60);

// 정적 파일
app.serveStatic("/static", "./public");

// WebSocket
app.ws("/ws", wsHandler);

// 에러 핸들러
app.onError(404).html("<h1>404 Not Found</h1>");
app.onError(500).html("<h1>500 Internal Server Error</h1>");

// 서버 시작
app.listen();
```

### Config 파일

첫 실행 시 `config/프로젝트명.conf` 파일이 자동 생성됩니다.

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
// 메서드, 경로, 버전
req.getMethod();   // "GET", "POST", ...
req.getPath();     // "/upload"
req.getVersion();  // "HTTP/1.1"

// 헤더
req.getHeader("Content-Type");  // std::optional<std::string>

// URL 파라미터(/user/:id)
req.getParam("id");             // std::optional<std::string> 

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

// 세션 데이터
req.session.set("key", "value");
req.session.get("key");         // std::optional<std::string>

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

### 세션

세션 미들웨어를 등록하면 자동으로 세션이 관리됩니다.

```cpp
app.session();

app.get("/profile").handle([](ynet::Request& req, ynet::Response& res) {
    // 세션에 데이터 저장
    req.session.set("user_id", "123");

    // 세션에서 데이터 조회
    auto user = req.session.get("user_id");
    if(user.has_value()) {
        res.json(R"({"user": ")" + user.value() + R"("})");
    } else {
        res.status(401).json(R"({"error": "not logged in"})");
    }
});
```

동작 방식:
- 첫 요청 시 세션 ID 자동 생성 + `Set-Cookie` 응답
- 이후 요청에서 Cookie의 세션 ID로 세션 복원
- 보안 플래그 자동 적용: `HttpOnly`, `Secure`, `SameSite=Strict`
- 기본 TTL 1시간, 매 요청마다 갱신 (sliding expiration)
- 잘못되거나 만료된 세션은 자동으로 새 세션 생성

### 캐시

기본 LRU 캐시가 적용됩니다. 정책을 교체할 수 있습니다.

```cpp
// 기본 (LRU)
ynet::Cache cache(1024);

// 커스텀 정책
class MyPolicy : public ynet::CachePolicy {
public:
    void evict(std::unordered_map<std::string, ynet::CacheEntry>& entries) override {
        // 커스텀 eviction 로직
    }
};

ynet::Cache cache(1024, std::make_unique<MyPolicy>());
```

config 파일에서 최대 캐시 수를 설정할 수 있습니다:
```
max_cache=2048
```

### 미들웨어

```cpp
// 내장 미들웨어
app.cors("*");                  // CORS
app.logger();                   // 요청 로거
app.csrf();                     // CSRF 토큰 검증
app.sanitizer();                // 입력 검증
app.secureHeaders();            // 보안 헤더
app.session();                  // 세션 관리
app.rateLimit(100, 60);         // 60초에 100번 제한

// 커스텀 미들웨어 (Server 직접 사용 시)
server.use([](ynet::Request& req, ynet::Response& res, ynet::Next next) {
    // 요청 전 처리
    next();
    // 요청 후 처리
});
```

### 파일 업로드

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
        ws.sendFrame("echo: " + msg, 0x1);  // 텍스트 프레임 에코
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

## 라이선스

MIT License
