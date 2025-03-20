//
// Created by nicolas.gerard on 2025-03-18.
//

#ifndef MESHSERIALIZATION_H
#define MESHSERIALIZATION_H
#include <vector>
#include <filesystem>
#include <crossguid/guid.hpp>

namespace Serialization{
    enum MeshAttributeType {
        VERTEX_POSITION,
        VERTEX_TEXCOORD,
        VERTEX_NORMAL,
        VERTEX_COLOR,
        VERTEX_TANGENT,

    };
    struct MeshSerializationData {
        uint32_t vertexCount;
        uint32_t indiceCount;
        std::vector<MeshAttributeType> attributeTypes;//attribute stored in the imported file , in order
        std::vector<std::byte> data;//all data for the mesh un interleaved
    };

    struct MeshMetaData {
        xg::Guid guid;
        std::string name;//mostly for debugging clarity
        //unsigned int numVertices;
        //unsigned int numIndices;
        std::vector<MeshAttributeType> importedAttributes;//it is possible to import non provided attribute, they will be filled to a default value but a warning will be fired
    };
    bool MeshAttributeTypeToString(MeshAttributeType type, std::string& str);
    bool MeshAttributeTypeFromString(const std::string& string, MeshAttributeType& type);
    bool ImportMeshAsset(const std::filesystem::path &filePath, const std::filesystem::path &destination, MeshMetaData &metaData);
    bool ImportMeshAssetData(const std::filesystem::path &filePath, MeshSerializationData &meshData, const Serialization::MeshMetaData &metaData);
    bool DeserializeMeshAsset(const std::filesystem::path &filePath,MeshSerializationData& data);
    void UpdateMeshMetaData(const std::filesystem::path &filePath, MeshMetaData &metaData);
    void ReadMeshMetaData(const std::filesystem::path &filePath, MeshMetaData& data);
    void CreateMeshMetaData(const std::filesystem::path &filePath, MeshMetaData& data);
}
#endif //MESHSERIALIZATION_H
