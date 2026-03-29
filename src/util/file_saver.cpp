#include <filesystem>
#include <fstream>
#include <linux/limits.h>
#include "ynet/util/file_saver.h"

namespace ynet {
    bool saveFile(const Part& part, const std::string& dir) {
        std::string filename = std::filesystem::path(part.filename).filename().string();
        if(filename.empty()) return false;

        char dir_resolved[PATH_MAX];
        if(!realpath(dir.c_str(), dir_resolved)) return false;

        std::string fullpath = std::string(dir_resolved) + "/" + filename;

        int count = 0;
        while(std::filesystem::exists(fullpath)) {
            count++;
            size_t dot = filename.rfind('.');
            std::string name = filename.substr(0, dot);
            std::string ext = filename.substr(dot);
            fullpath = std::string(dir_resolved) + "/" + name + "_" + std::to_string(count) + ext;
        }

        char file_resolved[PATH_MAX];
        if(realpath(fullpath.c_str(), file_resolved)) {
            if(!std::string(file_resolved).starts_with(std::string(dir_resolved))) return false;
        }

        std::ofstream ofs(fullpath, std::ios::binary);
        if(!ofs.is_open()) return false;
        ofs.write(part.data, part.data_len);
        return !ofs.fail();
    }
}
