//
// Created by nicolas.gerard on 2025-05-06.
//

#include "TextureSerialization.h"

#include <cassert>
#include <cmath>

#include "MeshSerialization.h"
#define STB_IMAGE_IMPLEMENTATION
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

    std::ofstream SerializedFile;
    SerializedFile.open(destination.string() + "/" + metaData.guid.str(),std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    SerializedFile.write(reinterpret_cast<char*>(&data.format), sizeof(VkFormat));
    SerializedFile.write(reinterpret_cast<char*>(data.dimensions.data()), sizeof(unsigned int) * 2);
    SerializedFile.write(reinterpret_cast<char*>(&data.MipMapCount), sizeof(unsigned short));
    SerializedFile.write(reinterpret_cast<char*>(&data.data), dataSize);
    SerializedFile.close();

    //meta data
    metaData.name = filepath.filename().string();
    metaData.dimensions = data.dimensions;
    metaData.format = data.format;
    metaData.MipMapCount = data.MipMapCount;
    metaData.guid = xg::newGuid();
    SerializedFile.open(filepath.string() + ".meta", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    SerializedFile << "GUID: " << std::endl << metaData.guid << std::endl;
    SerializedFile << "Name: " << std::endl << metaData.name << std::endl;
    SerializedFile << "Dimensions: " << std::endl << data.dimensions[0] <<
        std::endl << data.dimensions[1] << std::endl;
    SerializedFile << "Format: " << std::endl << data.format << std::endl;
    SerializedFile << "MipMap : " << std::endl <<data.MipMapCount << std::endl;
    SerializedFile.close();

    //also missing some kind of validation
    return true;
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

bool Serialization::DeserializeTextureAsset(const std::filesystem::path &filepath,
                                            Serialization::TextureSerializationData &Data) {
    assert(!filepath.has_extension());
    std::ifstream ifs(filepath, std::ios_base::in | std::ios_base::binary);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file, asset might not have been imported." << std::endl;
        return false;
    }
    //could memcpy all meta info in one go, this is just for clarity
    char Reader[std::max({sizeof(Data.format), sizeof(Data.dimensions),sizeof(Data.MipMapCount)})];
    //format
    ifs.read(Reader, sizeof(Data.format));
    Data.format = *reinterpret_cast<VkFormat*>(Reader);
    //dimension
    ifs.read(Reader, sizeof(Data.dimensions));
    memcpy(Data.dimensions.data(),Reader,sizeof(Data.dimensions));
    //mip map
    ifs.read(Reader, sizeof(Data.MipMapCount));
    Data.MipMapCount = *reinterpret_cast<unsigned short*>(Reader);
    //data
    const unsigned int byteCount = Data.dimensions[0] * Data.dimensions[1] * 4;//temp, this is the format stb imports
    Data.data.resize(byteCount);
    ifs.read(reinterpret_cast<std::istream::char_type *>(Data.data.data()), byteCount);
    return true;//also no validation in this case
}


