#pragma once

#include <openssl/ssl.h>
#include <string>
namespace ynet {
    class TlsContext {
        private:
            SSL_CTX* ctx;
        public:
            TlsContext(const TlsContext&) = delete;
            TlsContext& operator=(const TlsContext&) = delete;
            TlsContext(TlsContext&& other) noexcept : ctx(other.ctx) { other.ctx = nullptr; }
            TlsContext& operator=(TlsContext&& other) noexcept {
                if(this != &other) {
                    if(ctx) SSL_CTX_free(ctx);
                    ctx = other.ctx;
                    other.ctx = nullptr;
                }
                return *this;
            }
            TlsContext(const std::string& cert_path, const std::string& key_path);
            SSL* wrap(int fd);
            ~TlsContext();
    };
}
