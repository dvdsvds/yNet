#include <fstream>
#include <linux/limits.h>
#include <sstream>
#include "ynet/core/static_file.h"
#include "ynet/util/mime.h"

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
        std::string root_str = std::string(root_resolved) + "/";
        if(!std::string(resolved).starts_with(root_str)) return false;

        std::ifstream file(resolved, std::ios::binary);
        if(!file.is_open()) return false;

        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        if(size > 50 * 1024 * 1024) {
            res.status(413).body("File too large");
            return true;
        }
        file.seekg(0, std::ios::beg);

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        std::string mimetype = getMimeType(resolved);
        res.status(200).header("Content-Type", mimetype).body(content);
        return true;
    }
    return false;
}
