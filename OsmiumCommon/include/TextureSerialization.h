//
// Created by nicolas.gerard on 2025-05-06.
//

#ifndef TEXTURESERIALIZATION_H
#define TEXTURESERIALIZATION_H
#include <array>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include "crossguid/guid.hpp"


namespace Serialization {
    struct TextureSerializationData {
        std::array<unsigned int, 2> dimensions;
        VkFormat format;
        unsigned short MipMapCount;
        //compression data would be added here
        std::vector<std::byte> data;
    };
    struct TextureMetaData {
        xg::Guid guid;

        std::string name;
        std::array<unsigned int, 2> dimensions;
        VkFormat format;
        unsigned short MipMapCount;
    };
    bool ImportTextureAsset(const std::filesystem::path& filepath,const std::filesystem::path &destination, Serialization::TextureMetaData &metaData);
    bool ReadTextureMetaData(const std::filesystem::path& filepath, Serialization::TextureMetaData &metaData);
}




#endif //TEXTURESERIALIZATION_H
