# yNet

C++로 만든 경량 HTTP/HTTPS 프레임워크

**[English](README_en.md)**

---

## 소개

yNet은 C++20 기반의 경량 HTTP/HTTPS 프레임워크입니다. 라우팅, 미들웨어, 보안, 유틸리티까지 웹 서버에 필요한 핵심 기능을 제공합니다.

## 기술 스택

- **언어**: C++20
- **빌드**: CMake 3.20+
- **TLS**: OpenSSL
- **표준**: POSIX (Linux)

## 주요 기능

### HTTP 서버 코어
라우터 기반 요청 처리, 미들웨어 체인, 요청/응답 파싱, TCP 리스너 및 커넥션 관리

### 보안
HTTPS (TLS), CORS, CSRF 토큰 검증, 세션 관리, Rate Limiter, 입력 검증 (Sanitizer), 보안 헤더 자동 적용

### 유틸리티
JSON 파서/직렬화, URL 인코딩/디코딩/쿼리스트링 파싱, MIME 타입 매핑, TCP 클라이언트, 요청 로거, SHA-256 해싱

## 프로젝트 구조

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

## 라이선스

BSL-1.0 (Boost Software License)

