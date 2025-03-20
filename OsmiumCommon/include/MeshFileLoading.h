//
// Created by nicolas.gerard on 2025-02-05.
//
/*
 * This file is linked from Core, it should be fixed
 */
#ifndef MESHLOADING_H
#define MESHLOADING_H
#include <tiny_obj_loader.h>
#include <vector>
#include <string>
#include <fstream>
#include "DefaultVertex.h"
namespace MeshFileLoading{
//#define USE_CUSTOM_OBJ_LOADER

#ifdef USE_CUSTOM_OBJ_LOADER
    inline bool CustomLoadObj(const std::filesystem::path& path,std::vector<DefaultVertex>& vertices,std::vector<uint32_t>& indices, std::string * err) {
        vertices.clear();
        indices.clear();
        std::ifstream file(path.string());
        if (!file.is_open()) {
            *err = "Error opening file " + path.string();
            return false;
        }
        size_t lineNum = 0;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::ivec3> indicesData;
        std::string line;
        while (!file.eof()) {
            std::getline(file, line);
            lineNum++;
            //tiny obj trims the line here, I'm going to try to detect when the line has no relevant data
            char* current = &line[line.find_first_not_of('\t')];//switch to the more broadly applicalt strspn
            if (line.empty() || current[0] == '#') {//empty or comment
                continue;
            }
            if (current[0] == 's' || current[0] == 'u' || current[0] == 'o') {
                continue;//unssupported or unrelevant lines
            }
            //vertex
            if (current[0] == 'v' && std::isspace(current[1])) {//I'd rather not mix and match c and C++ here
                current += 2;
                glm::vec3 vert;
                //on failure, it shoudl simply return 0.0f
                vert[0] = strtof(current,&current);
                vert[1] = strtof(current,&current);
                vert[2] = strtof(current,&current);
                //the standard has an optional Uniform coordinate w value here

                positions.push_back(vert);
                //some extended version of the format might have more data in these lines, ignoring for now
                continue;
            }
            //normal
            if (current[0] == 'v' && current[1] == 'n' && std::isspace(current[2])) {
                current += 3;
                glm::vec3 norm;
                norm[0] = strtof(current,&current);
                norm[1] = strtof(current,&current);
                norm[2] = strtof(current,&current);

                normals.push_back(norm);
                continue;
            }
            //texcoord
            if (current[0] == 'v' && current[1] == 't' && std::isspace(current[2])) {
                current += 3;
                glm::vec2 tex;
                tex[0] = strtof(current,&current);
                tex[1] = strtof(current,&current);

                texCoords.push_back(tex);
                continue;
            }
            //face
            if (current[0] == 'f' && std::isspace(current[1])) {
                current += 2;
                glm::ivec3 ind;
                for (int i = 0; i < 3; ++i) {
                    ind[0] = strtol(current,&current,10);
                    if (current[0] == '/') {//has texcoord or normal
                        if (current[1] == '/') {
                            ind[1] = -1;//no texcoord
                            ++current;
                        }else
                            ind[1] = strtol(++current,&current,10);//tex coord

                        if (current[0] == '/')//has normal
                            ind[2] = strtol(++current,&current,10);
                        else
                            ind[2] = -1;

                        indicesData.push_back(ind);
                    }
                }
            }
        }
        file.close();
        vertices.reserve(indicesData.size());
        indices.reserve(indicesData.size());
        std::unordered_map<DefaultVertex, uint32_t> uniqueVertices {};
        for (glm::ivec3& i : indicesData) {
             auto vertex = DefaultVertex(positions[i[0]-1],
                 glm::vec3(1.0f),
                 i[1] == -1 ? glm::vec2(0.0f) :texCoords[i[1] -1],
                 i[2] == -1 ? glm::vec3(0.0f) : normals[i[2]-1]);
            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = uniqueVertices.size();
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
        return true;
    }
#endif
    inline void LoadFromObj(std::vector<DefaultVertex>& vertices,std::vector<uint32_t>& indices, const std::filesystem::path& path) {//using a default vertex format for now
        std::string warn,err;
#ifndef USE_CUSTOM_OBJ_LOADER
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;//this will probably hook into pipeline creation later

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
#else
        if (!CustomLoadObj(path,vertices, indices,&err)) {
            throw std::runtime_error(err);
        }
#endif


    }


}

#endif //MESHLOADING_H
