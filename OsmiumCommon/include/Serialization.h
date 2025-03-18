//
// Created by nicolas.gerard on 2025-03-18.
//

#ifndef MESHSERIALIZATION_H
#define MESHSERIALIZATION_H
#include <filesystem>
namespace Serialization {
    struct MeshData;
    bool ImportMeshAsset(const std::filesystem::path &filePath, const std::filesystem::path &destination);
    bool ImportMeshAsset(const std::filesystem::path &filePath, MeshData &meshData);
    bool DeserializeMeshAsset(const std::filesystem::path &filePath,MeshData& data);
}
#endif //MESHSERIALIZATION_H
