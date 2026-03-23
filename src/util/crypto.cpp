#include <cstdio>
#include <vector>
#include <ynet/util/crypto.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

namespace ynet {
    static std::string toHex(const unsigned char* data, size_t len) {
        std::string result;
        char buf[3];
        for(size_t i = 0; i < len; i++) {
            std::snprintf(buf, sizeof(buf), "%02x", data[i]); 
            result += buf;
        }
        return result;
    }

    std::string sha256(const std::string &input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
        return toHex(hash, SHA256_DIGEST_LENGTH);
    }

    std::string hmac_sha256(const std::string &key, const std::string &message) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int len = 0;
        HMAC(EVP_sha256(), key.c_str(), key.size(), reinterpret_cast<const unsigned char*>(message.c_str()), message.size(), hash, &len);
        return toHex(hash, len);
    }

    std::string random_hex(size_t length) {
        std::vector<unsigned char> buf(length);
        RAND_bytes(buf.data(), length);
        return toHex(buf.data(), length);
    }
}
