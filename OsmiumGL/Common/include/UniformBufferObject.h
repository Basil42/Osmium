//
// Created by Shadow on 1/27/2025.
//

#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H
#include <glm/mat4x4.hpp>

namespace Descriptors {
    struct UniformBufferObject {
        glm::mat4 model;
        //glm::mat4 normal;
    };
}
#endif //UNIFORMBUFFEROBJECT_H
