#include "ynet/util/mime.h"
#include <fstream>
#include <linux/limits.h>
#include <sstream>
#include <ynet/static_file.h>

using namespace ynet;

bool StaticFileServer::tryServe(const Request& req, Response& res) {
    std::string path = req.getPath();
    if(path.starts_with(url_prefix)) {
        auto rel = path.substr(url_prefix.size());
        std::string file_path = root_dir + rel;
        char resolved[PATH_MAX];
        if(!realpath(file_path.c_str(), resolved)) return false;

        char root_resolved[PATH_MAX];
        if(!realpath(root_dir.c_str(), root_resolved)) return false;
        if(std::string(resolved).starts_with(std::string(root_resolved)) == false) return false;

        std::ifstream file(resolved, std::ios::binary);
        if(!file.is_open()) return false;

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        std::string mimetype = getMimeType(resolved);
        res.status(200).header("Content-Type", mimetype).body(content);
        return true;
    }
    return false;
}
