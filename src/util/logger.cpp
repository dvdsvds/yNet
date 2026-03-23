#include "ynet/middleware.h"
#include "ynet/request.h"
#include "ynet/response.h"
#include <chrono>
#include <iostream>
#include <ctime>
#include <ynet/util/logger.h>

namespace ynet {
    Middleware Logger() {
        return [](const Request& req, Response& res, Next next) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            char time_str[20];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    
            next();
            std::string log = "[" + std::string(time_str) + "]" + " " + req.getMethod() + " " + req.getPath() + " " + std::to_string(res.getStatusCode()) + " " + req.getClientIP();
            std::cout << log << std::endl;
        };
    }
}
