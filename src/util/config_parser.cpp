#include <fstream>
#include <iostream>
#include <string>
#include <ynet/util/config_parser.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace ynet {
    Config loadConfig() {
        std::string dir_name = fs::current_path().filename().string();
        std::string conf_dir = "config";
        std::string conf_path = conf_dir + "/" + dir_name + ".conf";

        if(!fs::exists(conf_dir)) {
            fs::create_directories(conf_dir);
        }

        if(!fs::exists(conf_path)) {
            std::ofstream out(conf_path);
            out << "# " << dir_name << " configuration\n" 
                << "port=8080\n"
                << "bind=0.0.0.0\n"
                << "tls=off\n"
                << "cert=cert.pem\n"
                << "key=key.pem\n"
                << "max_body=1MB\n"
                << "max_upload=10MB\n"
                << "max_header=8KB\n"
                << "max_headers=64\n"
                << "header_timeout=5000\n"
                << "body_timeout=10000\n"
                << "max_connections=1024\n";
        }

        std::ifstream file(conf_path);
        if(!file.is_open()) {
            std::cerr << "[WARNING] cannot open " << conf_path << ", using defaults\n";
            return Config{};
        }

        std::string line;
        Config config;
        while(std::getline(file, line)) {
            if(line.empty() || line[0] == '#') continue; 
            size_t pos = line.find('=');
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if(key == "port") config.port = std::stoi(value);
            else if(key == "bind") config.bind_addr = value;
            else if(key == "tls") config.use_tls = (value == "on");
            else if(key == "cert") config.cert_path = conf_dir + "/" + value;
            else if(key == "key") config.key_path = conf_dir + "/" + value;
            else if(key == "max_body") config.max_body_size = parseSize(value);
            else if(key == "max_upload") config.max_upload_size = parseSize(value);
            else if(key == "max_header") config.max_header_size = parseSize(value);
            else if(key == "max_headers") config.max_headers = std::stoi(value);
            else if(key == "header_timeout") config.header_timeout_ms = std::stoi(value);
            else if(key == "body_timeout") config.body_timeout_ms = std::stoi(value);
            else if(key == "max_connections") config.max_connections = std::stoi(value);
        }
        return config;
    }

    size_t parseSize(const std::string& val) {
        size_t pos = val.find_first_not_of("0123456789");
        if(pos == std::string::npos) return std::stoul(val);

        size_t num = std::stoul(val.substr(0, pos));
        std::string unit = val.substr(pos);
        if(unit == "KB") {
            num = num * 1024;
        } else if(unit == "MB") {
            num = num * 1024 * 1024;
        } else if(unit == "GB") {
            num = num * 1024 * 1024 * 1024;
        } else {
            std::cerr << "[WARNING] unknown unit: "  << unit << ", treating as bytes\n";
        }

        return num;
    }
}
