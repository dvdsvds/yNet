#pragma once

#include "ynet/core/request.h"
#include "ynet/core/response.h"
#include <string>
namespace ynet {
    class StaticFileServer {
        private:
            std::string url_prefix;
            std::string root_dir;
        public:
            StaticFileServer(const std::string& prefix, const std::string& dir) 
                : url_prefix(prefix), root_dir(dir) {}
            bool tryServe(const Request& req, Response& res);
    };
}
