//
// Created by nicolas.gerard on 2025-03-19.
//
#define NOMINMAX //disables some ill named windows macros
#include "../include/Serialization.h"
#include <cassert>
#include <fstream>
#include <MeshSerialization.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <crossguid/guid.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "Memcheck.h"

/**
 * Update the meta data file for the source mesh asset, it contains, in plain text a GUID and import data
 * @param filePath path of source asset file
 */
void Serialization::UpdateMeshMetaData(const std::filesystem::path &filePath, Serialization::MeshMetaData &metaData) {
    std::string metapath = filePath.string();
    metapath.append(".meta");
    assert(std::filesystem::exists(metapath));
    xg::Guid g = metaData.guid;
    std::ofstream ofs(metapath, std::ios_base::out | std::ios_base::trunc);
    ofs << "GUID:" << std::endl <<  g << std::endl;
    ofs << "Type:" << std::endl << "Mesh" << std::endl;
    ofs << "Name:" << std::endl << metaData.name << std::endl;
    ofs << "Attributes:" <<std::endl;
    for (Serialization::MeshAttributeType attribute: metaData.importedAttributes) {
        std::string str;
        MeshAttributeTypeToString(attribute,str);
        ofs << str << std::endl;
    }
    ofs.close();
}

/**
 * Read the meta file associated to the provided source file into an output struct
 * @param filePath File path of the source file (not the meta file)
 * @param data reference to a Mesh data struct that will be filled by the function
 */
void Serialization::ReadMeshMetaData(const std::filesystem::path &filePath, MeshMetaData &data) {

    std::string metapath = filePath.string();
    metapath.append(".meta");
    std::ifstream ifs(metapath, std::ios_base::in);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file " << metapath << std::endl;
        return;
    }
    //I skip the lines meant for human users
    std::string line;
    std::getline(ifs, line);//guid
    std::getline(ifs, line);
    data.guid = xg::Guid(line.c_str());
    std::getline(ifs, line);//Type
    std::getline(ifs, line);//should be mesh
    assert(line == "Mesh");
    std::getline(ifs, line);//name
    std::getline(ifs, line);
    data.name = line;
    std::getline(ifs, line);//attributes
        MeshAttributeType attribute;
    while (std::getline(ifs, line)) {
        if (MeshAttributeTypeFromString(line, attribute)) {
            data.importedAttributes.push_back(attribute);
        }
    }
}

void Serialization::CreateMeshMetaData(const std::filesystem::path &filePath, MeshMetaData &data) {
    data.name = filePath.filename().string();
    data.guid = xg::newGuid();
    data.importedAttributes.clear();
    //default imported attributes
    data.importedAttributes.emplace_back(VERTEX_POSITION);
    data.importedAttributes.emplace_back(VERTEX_TEXCOORD);
    data.importedAttributes.emplace_back(VERTEX_NORMAL);
    std::ofstream ofs(filePath.string() + ".meta", std::ios_base::out | std::ios_base::trunc);
    ofs.close();
    UpdateMeshMetaData(filePath, data);
}


/**
 * Import a supported mesh file into a format that can be quickly loaded into OsmiumGL using the source file associated .meta file.
 * Updates or create a default .meta file on successful import and save the imported data into the resource folder.
 * @param filePath source file to import
 * @param destination folder to import to, usually the resource folder
 * @return success
 */
bool Serialization::ImportMeshAsset(const std::filesystem::path &filePath, const std::filesystem::path &destination, Serialization::MeshMetaData &metaData) {
    MeshSerializationData data{};
    if (!ImportMeshAssetData(filePath, data, metaData))
        return false;
    //where to document in an accessible way the struture of the exported file ?


    constexpr size_t attributeLimit = std::numeric_limits<size_t>::max() / sizeof(MeshAttributeType);
    uint32_t attributeCount = data.attributeTypes.size();
    if (attributeCount > attributeLimit) {
        std::cerr << "Too many attributes found, this is likely due to corrupted data" << std::endl;
        return false;
    }
    auto totalDataSize = data.data.size();
    if (totalDataSize >= std::numeric_limits<std::streamsize>::max()) {
        std::cerr << "mesh data is too large to be written to file. Likely due to corrupted data.";
        return false;
    }
    std::ofstream ofs(destination.string() + "/" + metaData.guid.str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open file " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to open file");
    }
    ofs.write(reinterpret_cast<char *>(&data.vertexCount),sizeof(data.vertexCount));
    ofs.write(reinterpret_cast<char *>(&data.indiceCount),sizeof(data.indiceCount));

    ofs.write(reinterpret_cast<char*>(&attributeCount) ,sizeof(attributeCount));
    ofs.write(reinterpret_cast<char*>(data.attributeTypes.data()), (sizeof(MeshAttributeType) * attributeCount));//warning soled with check but not detected, NOLINT(*-narrowing-conversions)
    ofs.write(reinterpret_cast<char*>(data.data.data()),totalDataSize);  // NOLINT(*-narrowing-conversions)
    ofs.close();
    return true;
}

/**
 * Import a supported mesh file to a struct that can be loaded into OsmiumGL, useful for runtime imports
 * @param filePath source file to import
 * @param meshData raw input of the import, useful for runtime import of supported types
 * @return success
 */
bool Serialization::ImportMeshAssetData(const std::filesystem::path &filePath, MeshSerializationData &meshData,const Serialization::MeshMetaData& metaData) {

    assert(filePath.extension() == ".obj");//this is the only mesh file type we support for now
    std::vector<uint32_t> indices;
    //these are the only attribute we support on obj for now
    //assuming positions are provided
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    std::string warn,err;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.string().c_str())) {
        std::cerr << warn + err << std::endl;
        return false;
    }
    std::unordered_map<size_t,uint32_t> uniqueVertices;//use for de-duplication


    bool _normalProvided = !attrib.normals.empty();
    bool _texCoordProvided = !attrib.texcoords.empty();
    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            glm::vec3 position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]};
            glm::vec3 normal = {
            _normalProvided ? attrib.normals[3 * index.normal_index + 0] : 0.0f,
            _normalProvided ? attrib.normals[3 * index.normal_index + 1] : 0.0f,
            _normalProvided ? attrib.normals[3 * index.normal_index + 2] : 1.0f};
            glm::vec2 texCoord{
            _texCoordProvided ? attrib.texcoords[2 * index.texcoord_index + 0] : 0.0f,
            _texCoordProvided ? attrib.texcoords[2 * index.texcoord_index + 1] : 0.0f};

            size_t hash = (std::hash<glm::vec3>()(position) ^ std::hash<glm::vec3>()(normal) << 1) >> 1 ^ std::hash<glm::vec2>()(texCoord) << 1;
            if (!uniqueVertices.contains(hash)) {
                uniqueVertices[hash] = static_cast<uint32_t>(uniqueVertices.size());
                positions.push_back(position);
                normals.push_back(normal);
                texCoords.push_back(texCoord);
            }
            indices.push_back(uniqueVertices[hash]);
        }
    }
    meshData.attributeTypes = metaData.importedAttributes;
    meshData.vertexCount = uniqueVertices.size();
    meshData.indiceCount = indices.size();

    std::vector<glm::vec3> tangents;
    bool importTangent = false;
    std::vector<glm::vec4> colors;
    bool importColor = false;

    unsigned int allocSize = 0;
    for (const auto& attribute : metaData.importedAttributes) {
        switch (attribute) {
            case VERTEX_POSITION:
                allocSize += sizeof(glm::vec3);
                break;
            case VERTEX_TEXCOORD:
                allocSize += sizeof(glm::vec2);
                break;
            case VERTEX_NORMAL:
                allocSize += sizeof(glm::vec3);
                break;
            case VERTEX_COLOR:
                allocSize += sizeof(glm::vec4);
                importColor = true;
                colors.resize(meshData.vertexCount);
                break;
            case VERTEX_TANGENT:
                allocSize += sizeof(glm::vec3);
                importTangent = true;
                tangents.resize(meshData.vertexCount);
                break;
            default: ;
        }
    }
    allocSize *= meshData.vertexCount;
    const auto indiceAllocSize = sizeof(uint32_t) * meshData.indiceCount;
    allocSize += indiceAllocSize;

    if (!CanAllocate(allocSize))return false;

    meshData.data.resize(allocSize);
    unsigned int offset = 0;

    for (const auto& attribute : metaData.importedAttributes) {
        switch (attribute) {
            case VERTEX_POSITION: {
                unsigned int positionsOffset = positions.size() * sizeof(glm::vec3);
                memcpy(meshData.data.data() + offset, positions.data(), positionsOffset);
                offset += positionsOffset;
                break;
            }
            case VERTEX_TEXCOORD: {
                unsigned int texCoordsOffset = texCoords.size() * sizeof(glm::vec2);
                memcpy(meshData.data.data() + offset, texCoords.data(), texCoordsOffset);
                offset += texCoordsOffset;
                break;
            }
            case VERTEX_NORMAL: {
                unsigned int normalsOffset = normals.size() * sizeof(glm::vec3);
                memcpy(meshData.data.data() + offset, normals.data(), normalsOffset);
                offset += normalsOffset;
                break;
            }
            case VERTEX_COLOR: {
                unsigned int colorsOffset = colors.size() * sizeof(glm::vec4);
                memcpy(meshData.data.data() + offset, colors.data(), colorsOffset);
                offset += colorsOffset;
                break;
            }
            case VERTEX_TANGENT: {
                unsigned int tangentsOffset = tangents.size() * sizeof(glm::vec3);
                memcpy(meshData.data.data() + offset, tangents.data(),tangentsOffset);
                offset += tangentsOffset;
                break;
            }
            default: {
                break;
            } ;
        }
        assert(offset < allocSize);//checking we didn't mess up when allocating
    }
    memcpy(meshData.data.data() + offset,indices.data(),indiceAllocSize);
    offset += indiceAllocSize;
    assert(offset == allocSize);

    return true;
}

bool Serialization::DeserializeMeshAsset(const std::filesystem::path &filePath, MeshSerializationData &data) {
    assert(!filePath.has_extension());//file should be named after it's GUID
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << filePath << std::endl;
        return false;
    }
    char uintReader[sizeof(uint32_t)];
    file.read(uintReader, sizeof(data.vertexCount));
    data.vertexCount = *reinterpret_cast<uint32_t*>(uintReader);
    file.read(uintReader, sizeof(data.indiceCount));
    data.indiceCount = *reinterpret_cast<uint32_t*>(uintReader);
    file.read(uintReader, sizeof(uint32_t));
    uint32_t attributeCount = *reinterpret_cast<uint32_t*>(uintReader);

    data.attributeTypes.resize(attributeCount);
    file.read(reinterpret_cast<char*>(data.attributeTypes.data()), attributeCount * sizeof(MeshAttributeType));
    unsigned int dataSize = 0;
    for (auto attribute : data.attributeTypes) {
        switch (attribute) {
            case VERTEX_POSITION:
                dataSize += sizeof(glm::vec3);
                break;
            case VERTEX_TEXCOORD:
                dataSize += sizeof(glm::vec2);
                break;
            case VERTEX_NORMAL:
                dataSize += sizeof(glm::vec3);
                break;
            case VERTEX_COLOR:
                dataSize += sizeof(glm::vec4);
                break;
            case VERTEX_TANGENT:
                dataSize += sizeof(glm::vec3);
                break;
            default:
                std::cerr << "Unknown attribute " << attribute << std::endl;
                return false;
                break;
        }
    }
    dataSize *= data.vertexCount;
    dataSize += sizeof(uint32_t) * data.indiceCount;
    data.data.resize(dataSize);
    file.read(reinterpret_cast<char*>(data.data.data()), dataSize);
    //not sure how to validate this
    return true;


}




bool Serialization::MeshAttributeTypeToString(MeshAttributeType type, std::string &str) {
    switch (type) {
        case VERTEX_POSITION:
            str = "POSITION";
            break;
        case VERTEX_TEXCOORD:
            str = "TEXCOORD";
            break;
        case VERTEX_NORMAL:
            str = "NORMAL";
            break;
        case VERTEX_COLOR:
            str = "COLOR";
            break;
        case VERTEX_TANGENT:
            str = "TANGENT";
            break;
        default:
            return false;
    }
    return true;
}

bool Serialization::MeshAttributeTypeFromString(const std::string &string, MeshAttributeType &type) {
    if (string == "POSITION") {
        type = VERTEX_POSITION;
        return true;
    }
    if (string == "TEXCOORD") {
        type = VERTEX_TEXCOORD;
        return true;
    }
    if (string == "NORMAL") {
        type = VERTEX_NORMAL;
        return true;
    }
    if (string == "COLOR") {
        type = VERTEX_COLOR;
        return true;
    }
    if (string == "TANGENT") {
        type = VERTEX_TANGENT;
        return true;
    }
    return false;
}
