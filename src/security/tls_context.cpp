#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>
#include <ynet/security/tls_context.h>

using namespace ynet;

TlsContext::TlsContext(const std::string& cert_path, const std::string& key_path) {
    ctx = SSL_CTX_new(TLS_server_method());
    if(ctx == nullptr) {
        throw std::runtime_error("SSL_CTX_new failed");
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    if(SSL_CTX_use_certificate_file(ctx, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(ctx);
        throw std::runtime_error("cert load failed");
    }
    if(SSL_CTX_use_PrivateKey_file(ctx, key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(ctx);
        throw std::runtime_error("key load failed");
    }
}

SSL* TlsContext::wrap(int fd) {
    SSL* ssl = SSL_new(ctx);
    if(ssl == nullptr) { throw std::runtime_error("SSL_new failed"); }
    SSL_set_fd(ssl, fd);
    return ssl;
}

TlsContext::~TlsContext() {
    SSL_CTX_free(ctx);
}
