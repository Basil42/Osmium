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

void MeshAsset::Load() {//loading to default format for now
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

std::mutex& MeshAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Mesh);
}