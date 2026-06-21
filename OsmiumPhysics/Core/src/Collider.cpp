//
// Created by Shadow on 2/4/2025.
//

#include "Collider.h"

auto Collider::setTransform(const glm::mat4 &transform) -> void {m_transform = &transform;}

void BoxCollider::getWorldVertices(std::array<glm::vec3, 8> &array) const {//could cache these for static geometry as well
    array = {
        glm::vec3(-size.x/2.0f, -size.y/2.0f, -size.z/2.0f),
        glm::vec3(size.x/2.0f, -size.y/2.0f, -size.z/2.0f),
        glm::vec3(-size.x/2.0f, -size.y/2.0f, size.z/2.0f),
        glm::vec3(size.x/2.0f, -size.y/2.0f, size.z/2.0f),
        glm::vec3(-size.x/2.0f, size.y/2.0f, -size.z/2.0f),
        glm::vec3(size.x/2.0f, size.y/2.0f, -size.z/2.0f),
        glm::vec3(-size.x/2.0f, size.y/2.0f, size.z/2.0f),
        glm::vec3(size.x/2.0f, size.y/2.0f, size.z/2.0f),
    };



    for (int i = 0; i < 8; i++) {
        const glm::vec4 transformedVertex = *m_transform * glm::vec4(array[i],1.0f);//maybe the array should be vec4
        assert(transformedVertex.w == 1.0f);
        array[i] = transformedVertex;
    }
}
