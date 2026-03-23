#pragma once

#include <cstdint>
#include <string>
namespace ynet {
    struct Config {
        uint16_t port = 8080; 
        std::string bind_addr = "0.0.0.0"; 
        size_t max_body_size = 1 * 1024 * 1024;
        size_t max_header_size = 8192;
        int max_headers = 64;
        int header_timeout_ms = 5000;
        int body_timeout_ms = 10000;
        int max_connections = 1024;
        bool use_tls = false;
        std::string cert_path;
        std::string key_path;
    };
}
