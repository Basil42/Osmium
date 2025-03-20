#include "Serialization.h"

#include <fstream>

#include <assert.h>


xg::Guid Serialization::GetGUIDFromMetaData(const std::filesystem::path &path) {
    std::ifstream meta(path.string() + ".meta", std::ios::in);
    if (!meta.is_open()) {
        std::cerr << path.string() + "is not imported";
        return xg::Guid();//return empty guid
    }
    std::string line;
    std::getline(meta, line);
    assert(line == "GUID:");//checking the header is correct
    std::getline(meta, line);
    return xg::Guid(line.c_str());
}
