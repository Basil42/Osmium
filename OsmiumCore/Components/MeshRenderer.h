//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

class Material;
template<class VertexType, typename VertexType::vertexFormat n = 0>
class MeshRenderer{
    void* meshData;
    size_t meshDataSize;
    //material class pointer
    Material* material;



};



#endif //MESHRENDERER_H
