
#ifndef TUTORIALVERTEX_H
#define TUTORIALVERTEX_H
#include <array>
#include <glm\glm.hpp>
#include <vulkan/vulkan_core.h>
struct DefaultVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoordinates;
    //glm::vec3 testPadding;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(DefaultVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription,3> attributeDescriptions = {};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(DefaultVertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(DefaultVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(DefaultVertex,texCoordinates);

        return attributeDescriptions;
    }
    bool operator==(const DefaultVertex& other) const {
        return position == other.position && color == other.color &&
            texCoordinates == other.texCoordinates;
    }
};
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
namespace std {
    template <> struct hash<DefaultVertex> {
        size_t operator()(const DefaultVertex& t) const noexcept {
            return ((hash<glm::vec3>()(t.position) ^ (hash<glm::vec3>()(t.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(t.texCoordinates) << 1);// ^ (hash<glm::vec3>()(t.testPadding) << 1);
        }
    };
}
#endif //TUTORIALVERTEX_H
