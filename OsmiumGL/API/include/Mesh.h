//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef MESH_H
#define MESH_H
#include <string>


typedef unsigned long MeshHandle;
typedef unsigned long MaterialHandle;
//Utility class for handling meshes
class Mesh {

    MeshHandle meshHandle;

    public:
    Mesh* LoadMesh(std::string fileName);
};



#endif //MESH_H
