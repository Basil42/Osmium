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
  glm::vec3 color;
  glm::vec2 texCoords;
  glm::vec3 normal;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {
    .binding = 0,
    .stride = sizeof(BlinnPhongVertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,};
    return bindingDescription;
  }
  static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(BlinnPhongVertex, position);


    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(BlinnPhongVertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(BlinnPhongVertex, texCoords);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(BlinnPhongVertex, normal);

    return attributeDescriptions;
  }
  bool operator==(const BlinnPhongVertex& other) const {
    return position == other.position && color == other.color && texCoords == other.texCoords &&
      normal == other.normal;
  }
};
#endif //BLINNPHONGVERTEX_H
