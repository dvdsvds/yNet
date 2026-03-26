#include <fstream>
#include <ynet/util/file_saver.h>

namespace ynet {
    bool saveFile(const Part& part, const std::string& path) {
        std::string filename = part.filename;
        size_t pos1 = filename.rfind('/');
        size_t pos2 = filename.rfind('\\');
        if(pos1 == std::string::npos && pos2 == std::string::npos) {
            std::ofstream ofs(path + "/" + filename, std::ios::binary);
            if(!ofs.is_open()) {
                return false;
            }
            ofs.write(part.data, part.data_len);
            if(ofs.fail()) {
                return false;
            }
            ofs.close();
            return true;
        } else {
            size_t p1 = (pos1 == std::string::npos) ? 0 : pos1;
            size_t p2 = (pos2 == std::string::npos) ? 0 : pos2;
            size_t bigger = (p1 > p2) ? p1 : p2;
            std::string organize_name = filename.substr(bigger + 1);
            std::ofstream ofs(path + "/" + organize_name, std::ios::binary);
            if(!ofs.is_open()) {
                return false;
            }
            ofs.write(part.data, part.data_len);
            if(ofs.fail()) {
                return false;
            }
            ofs.close();
            return true;
        }
    }
}
