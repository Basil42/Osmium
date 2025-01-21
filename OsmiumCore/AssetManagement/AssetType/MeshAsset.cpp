//
// Created by Shadow on 12/10/2024.
//

#include "MeshAsset.h"

#include <tiny_obj_loader.h>
#include <unordered_map>
#include <iostream>
#include <DefaultVertex.h>

#include "OsmiumGL_API.h"
#include "../AssetManager.h"
#include "../../Base/ResourceManager.h"

void MeshAsset::LoadFromObj(std::vector<DefaultVertex>& vertices,std::vector<uint32_t>& indices) {//using a default vertex format for now
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;//this will probably hook into pipeline creation later
    std::string warn,err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str())) {
        throw std::runtime_error(warn + err);//do some error handling instead of crashing directly
    }
    std::unordered_map<DefaultVertex, uint32_t> uniqueVertices {};
    bool useTextCoord = !attrib.texcoords.empty();
    for(const auto& shape : shapes) {
        for(const auto& index : shape.mesh.indices) {
            DefaultVertex vertex{
                .position = {
                    attrib.vertices[3* index.vertex_index + 0],
                    attrib.vertices[3* index.vertex_index + 1],
                    attrib.vertices[3* index.vertex_index + 2]},
                .color = {1.0f,1.0f,1.0f},
                .texCoordinates = {
                    useTextCoord ? attrib.texcoords[2 * index.texcoord_index +0]: 0.0f,
                     useTextCoord ? 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] : 0.0f},
                };

            if(!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }



}
[[deprecated("Use version with vertex format instead")]]
void MeshAsset::loadMeshToDeprecatedFormat() {
    if(AssetManager::isAssetLoaded(id)) {
        std::cout << "trying to load asset that is already loaded" << std::endl;
        return;
    }

    auto fileExtension = path.extension();
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices;
    //check that the file extension is supported
    if(fileExtension == ".obj") {
        //obj loading here, might encapsulate later for more formats
        LoadFromObj(vertices, indices);

    }else {
        throw std::runtime_error("File extension " + fileExtension.string() + "  is not supported");
    }

    OsmiumGL::LoadMeshWithDefaultFormat(MeshHandle
                                        ,vertices,
                                        indices);
}


void MeshAsset::Load() {
    if (AssetManager::isAssetLoaded(id)) {
        std::cout << "trying to load asset that is already loaded" << std::endl;
        return;
    }
    auto fileExtension = path.extension();
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices;
    //check that the file extension is supported
    if(fileExtension == ".obj") {
        //obj loading here, might encapsulate later for more formats
        LoadFromObj(vertices, indices);

    }else {
        throw std::runtime_error("File extension " + fileExtension.string() + "  is not supported");
    }

    std::vector<VertexBufferDescriptor> buffersDescriptors;
    unsigned int vertexCount = vertices.size();
    std::allocator<std::byte> allocator;
    unsigned int allocationSize = 0;
    if (POSITION & vertexAttributeFlags) {
        allocationSize += sizeof(DefaultVertex::position);
    }
    if (TEXCOORD0 & vertexAttributeFlags) allocationSize += sizeof(DefaultVertex::texCoordinates);
    if (NORMAL & vertexAttributeFlags) allocationSize += sizeof(DefaultVertex::normal);

    //there are probably nicer de interleaving algorithm somewhere
    std::vector<glm::vec3> positions(vertexCount);
    std::vector<glm::vec2> texcoords(vertexCount);
    std::vector<glm::vec3> normals(vertexCount);
    //doing everything in one loop is likely better cache wise
    for(unsigned int i = 0; i < vertices.size(); i++) {
        positions[i] = vertices[i].position;
        texcoords[i] = vertices[i].texCoordinates;
        normals[i] = vertices[i].normal;
    }
    std::byte *buffer = allocator.allocate(allocationSize);
    unsigned int offset = 0;
    //I can probably turn that into a loop, everything is allignment 4
    if (POSITION & vertexAttributeFlags) {
        unsigned int totalSize
        = sizeof(DefaultVertex::position) * vertexCount;
        memcpy(buffer + offset, positions.data(), totalSize);
        buffersDescriptors.push_back({.AttributeStride = sizeof(DefaultVertex::position), .data = buffer + offset,.attribute = POSITION});
        offset += totalSize;
    }
    if (TEXCOORD0 & vertexAttributeFlags) {
        unsigned int totalSize
        = sizeof(DefaultVertex::texCoordinates) * vertexCount;
        memcpy(buffer + offset, texcoords.data(), totalSize);
        buffersDescriptors.push_back({.AttributeStride = sizeof(DefaultVertex::texCoordinates), .data = buffer + offset,.attribute = TEXCOORD0});
        offset += totalSize;
    }
    if (NORMAL & vertexAttributeFlags) {
        unsigned int totalSize
        = sizeof(DefaultVertex::normal) * vertexCount;
        memcpy(buffer + offset, normals.data(), totalSize);
        buffersDescriptors.push_back({.AttributeStride = sizeof(DefaultVertex::normal), .data = buffer + offset,.attribute = NORMAL});
        offset += totalSize;
    }
    OsmiumGL::LoadMesh(MeshHandle,buffer,vertexCount,buffersDescriptors,vertexAttributeFlags, customAttributeFlags, indices);
}

std::mutex& MeshAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Mesh);
}

unsigned long MeshAsset::GetMeshHandle() const {
    return MeshHandle;
}


void MeshAsset::Unload() {
    OsmiumGL::UnloadMesh(MeshHandle);
}

MeshAsset::MeshAsset(const std::filesystem::path &path) : Asset(path){
    MeshHandle = -1;// starts unloaded
    type = mesh;
}
