//
// Created by nicolas.gerard on 2025-05-06.
//

#include "TextureSerialization.h"

#include <cassert>
#include <cmath>

#include "MeshSerialization.h"
#include <stb_image.h>
#include <fstream>


bool Serialization::ImportTextureAsset(const std::filesystem::path &filepath, const std::filesystem::path &destination,
                                       Serialization::TextureMetaData &metaData) {
    TextureSerializationData data{};
    //import
    if (!(filepath.extension() == ".jpg" || filepath.extension() == ".jpeg" || filepath.extension() == ".png")) {
        std::cerr << "Trying to import an unsupported texture type." << std::endl;
        return false;
    }
    int width, height, channels;
    stbi_uc* pixels = stbi_load(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    unsigned int dataSize = width * height * 4;//stb_image return R8G8B8A8 pixels
    if (!pixels) {
        std::cerr << "Failed to load texture." << std::endl;
        return false;
    }
    //serialization Data
    data.data.resize(dataSize);
    memcpy(data.data.data(), pixels, dataSize);
    data.format = VK_FORMAT_R8G8B8A8_SRGB;
    data.dimensions[0] = width;
    data.dimensions[1] = height;
    data.MipMapCount = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    //meta data
    metaData.name = filepath.filename().string();
    metaData.dimensions = data.dimensions;
    metaData.format = data.format;
    metaData.MipMapCount = data.MipMapCount;
    metaData.guid = xg::newGuid();







}

bool Serialization::ReadTextureMetaData(const std::filesystem::path &filepath,
    Serialization::TextureMetaData &metaData) {
    std::string metaPath = filepath.string();
    metaPath.append(".meta");
    std::ifstream ifs(metaPath,std::ios_base::in);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open meta file." << std::endl;
        return false;
    }
    std::string line;
    std::getline(ifs, line);//guid header
    std::getline(ifs, line);//guid value
    metaData.guid = xg::Guid(line.c_str());
    std::getline(ifs, line);//Type
    std::getline(ifs, line);//should be Texture
    assert(line == "Texture");
    std::getline(ifs, line);//name
    std::getline(ifs, line);
    metaData.name = line;
    std::getline(ifs, line);//dimensions
    std::getline(ifs, line);//width
    metaData.dimensions[0] = std::stoi(line);
    std::getline(ifs, line);//height
    metaData.dimensions[1] = std::stoi(line);
    std::getline(ifs, line);//format
    std::getline(ifs, line);//seems like a large overhead to make that human-readable
    metaData.format = static_cast<VkFormat>(std::stoi(line));
    std::getline(ifs, line);//mipmapCount
    std::getline(ifs, line);
    metaData.MipMapCount = std::stoi(line);

    //ideally there shoudl be some kind of check that the data was valid
    return true;

}


