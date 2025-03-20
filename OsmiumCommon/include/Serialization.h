//
// Created by nicolas.gerard on 2025-03-18.
//

#ifndef SERIALIZATION_H
#define SERIALIZATION_H
#include <filesystem>

#include "crossguid/guid.hpp"

namespace Serialization {
    xg::Guid GetGUIDFromMetaData(const std::filesystem::path & path);
}
#endif //SERIALIZATION_H
