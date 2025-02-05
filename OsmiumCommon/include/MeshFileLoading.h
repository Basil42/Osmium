//
// Created by nicolas.gerard on 2025-02-05.
//

#ifndef MESHLOADING_H
#define MESHLOADING_H
#include <tiny_obj_loader.h>
#include <vector>

#include "DefaultVertex.h"

namespace MeshFileLoading{
    inline void LoadFromObj(std::vector<DefaultVertex>& vertices,std::vector<uint32_t>& indices, const std::filesystem::path& path) {//using a default vertex format for now
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
                    .normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]},
                        };

                if(!uniqueVertices.contains(vertex)) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }



    }
}

#endif //MESHLOADING_H
