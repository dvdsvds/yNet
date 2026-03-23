#pragma once

#include <openssl/ssl.h>
#include <string>
namespace ynet {
    class TlsContext {
        private:
            SSL_CTX* ctx;
        public:
            TlsContext(const std::string& cert_path, const std::string& key_path);
            SSL* wrap(int fd);
            ~TlsContext();
    };
}
