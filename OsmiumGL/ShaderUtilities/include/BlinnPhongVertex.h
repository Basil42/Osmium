//
// Created by nicolas.gerard on 2025-01-08.
//

#ifndef BLINNPHONGVERTEX_H
#define BLINNPHONGVERTEX_H
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "DefaultVertex.h"

struct BlinnPhongVertex {
  glm::vec3 position;
  glm::vec2 texCoords;
  glm::vec3 normal;

  static std::array<VkVertexInputBindingDescription, 3> getBindingDescription() {
    auto result = std::array<VkVertexInputBindingDescription, 3>();
    result[0].binding = 0;
    result[0].stride = sizeof(position);
    result[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    result[1].binding = 1;
    result[1].stride = sizeof(texCoords);
    result[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    result[2].binding = 2;
    result[2].stride = sizeof(normal);
    result[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return result;
  }
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(BlinnPhongVertex, position);

    attributeDescriptions[1].binding = 1;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(BlinnPhongVertex, texCoords);

    attributeDescriptions[2].binding = 2;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(BlinnPhongVertex, normal);

    return attributeDescriptions;
  }
  bool operator==(const BlinnPhongVertex& other) const {
    return position == other.position && texCoords == other.texCoords &&
      normal == other.normal;
  }
};
#endif //BLINNPHONGVERTEX_H
