//
// Created by Shadow on 1/22/2025.
//

#include "DefaultSceneDescriptorSets.h"

#include <stdexcept>

#include "BlinnPhongVertex.h"
#include "BlinnPhongVertex.h"
#include "BlinnPhongVertex.h"
#include "BlinnPhongVertex.h"
#include "Core.h"
#include "UniformBufferObject.h"

VkPipelineLayout DefaultSceneDescriptorSets::GetCameraPipelineLayout() const {
    return GlobalMainPipelineLayout;
}

VkPipelineLayout DefaultSceneDescriptorSets::GetLitPipelineLayout() const {
    return LitPipelineLayout;
}

void DefaultSceneDescriptorSets::CreateDefaultDescriptorPool(const VkDevice _device) {
    std::array<VkDescriptorPoolSize, BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT> sizes{};


    sizes[0] = VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    MAX_FRAMES_IN_FLIGHT * 2);

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *2),
        .poolSizeCount = static_cast<uint32_t>(BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT),
        .pPoolSizes = sizes.data()
    };

    if (vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for built in scene descriptors!");
    }
}

void DefaultSceneDescriptorSets::CreateDefaultDescriptorLayouts(const VkDevice _device) {
    //layout
    //directional light
    constexpr VkDescriptorSetLayoutBinding directionalLightLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr};



    VkDescriptorSetLayoutCreateInfo  dirLightDescriptorLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &directionalLightLayoutBinding,
    };

    if (vkCreateDescriptorSetLayout(_device, &dirLightDescriptorLayoutCreateInfo, nullptr, &litDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buitl in descriptor set layout!");
    }
    //view matrix, I could also reuse the struct, but thsi is clearer
    constexpr VkDescriptorSetLayoutBinding viewMatrixLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr};

    VkDescriptorSetLayoutCreateInfo viewMatrixDescriptorLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &viewMatrixLayoutBinding,};

    if (vkCreateDescriptorSetLayout(_device,&viewMatrixDescriptorLayoutCreateInfo,nullptr,&mainCameraDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create view matrix built in descriptor set layout!");
    }
}

void DefaultSceneDescriptorSets::CreateDescriptorSets(const VkDevice _device,
                                                                      const VmaAllocator allocator,
                                                                      const OsmiumGLInstance &GLInstance,
                                                                      const VkDescriptorSetLayout &
                                                                      descriptor_set_layout,
                                                                      std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> &descriptor_sets,
                                                                      std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> &uniformBuffers,
                                                                      std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> &allocations,
                                                                      std::array<void*,MAX_FRAMES_IN_FLIGHT> &mappedSource,
                                                                      const size_t uniformSize
                                                                      ) {
    std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> descriptorSetLayouts = {descriptor_set_layout, descriptor_set_layout};
    //sets (these should be global values and should not require to be rebouind during a frame
    VkDescriptorSetAllocateInfo dirLightDescriptorSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = descriptorSetLayouts.data()
    };
    if (vkAllocateDescriptorSets(_device,&dirLightDescriptorSetAllocateInfo,descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for directional light!");
    }
    //buffers
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        const VkDeviceSize bufferSize = uniformSize;

        GLInstance.createBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VMA_MEMORY_USAGE_AUTO,
                                uniformBuffers[i],
                                allocations[i], VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        //not very happy that I'd need it
        auto result = vmaMapMemory(allocator,allocations[i],&mappedSource[i]);//I'm uncomfortable with the fact that I do not allocate memory to these void pointers, but mapping might make it safe
        assert(result == VK_SUCCESS);

    }
    //writes, maybe object should do their own ?
     for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
         VkDescriptorBufferInfo descriptorBufferInfo = {
             .buffer = uniformBuffers[i],
             .offset = 0,
             .range = uniformSize};

         std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
         descriptorWrites[0] = {
             .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
             .dstSet = descriptor_sets[i],
             .dstBinding = 0,
             .dstArrayElement = 0,
             .descriptorCount = 1,
             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
             .pImageInfo = nullptr,
             .pBufferInfo = &descriptorBufferInfo,
             .pTexelBufferView = nullptr};
         vkUpdateDescriptorSets(_device,static_cast<uint32_t>(descriptorWrites.size()),descriptorWrites.data(),0, nullptr);
    }
}

DefaultSceneDescriptorSets::DefaultSceneDescriptorSets(const VkDevice _device,const VmaAllocator allocator,OsmiumGLInstance &GLInstance) : device(_device){
    CreateDefaultDescriptorPool(_device);
    CreateDefaultDescriptorLayouts(_device);
    Allocator = allocator;
    //directional light
    CreateDescriptorSets(_device, allocator, GLInstance,
                         litDescriptorSetLayout,
                         directionalLightDescriptorSets,
                         directionalLightUniformBuffers,
                         directionalLightAllocations,
                         directionalLightBufferMappedSources,
                         sizeof(DirLightUniform));



    //camera
    CreateDescriptorSets(_device, allocator, GLInstance,
                         mainCameraDescriptorSetLayout,
                         mainCameraViewMatrixDescriptorSets,
                         mainCameraViewMatrixUniformBuffers,
                         mainCameraViewMatrixAllocations,
                         mainCameraViewMatrixMappedSource,
                         sizeof(CameraUniform));

    CreateGlobalPipelineLayout(_device);
    CreateLitPipelineLayout(_device);
}

DefaultSceneDescriptorSets::~DefaultSceneDescriptorSets() {
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        vmaUnmapMemory(Allocator,directionalLightAllocations[i]);
        vmaDestroyBuffer(Allocator,directionalLightUniformBuffers[i],directionalLightAllocations[i]);
        vmaUnmapMemory(Allocator,mainCameraViewMatrixAllocations[i]);
        vmaDestroyBuffer(Allocator,mainCameraViewMatrixUniformBuffers[i],mainCameraViewMatrixAllocations[i]);
    }
    vkDestroyDescriptorPool(device,descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device,litDescriptorSetLayout,nullptr);
    vkDestroyDescriptorSetLayout(device,mainCameraDescriptorSetLayout,nullptr);

    vkDestroyPipelineLayout(device,GlobalMainPipelineLayout,nullptr);
    vkDestroyPipelineLayout(device,LitPipelineLayout,nullptr);

}

void DefaultSceneDescriptorSets::UpdateDirectionalLight(const DirLightUniform &updatedValue, const unsigned int currentImage) {
    directionalLightValue = updatedValue;
    //I'll update all frames manually for now, but I feel like this will cause some problems if I want to use it dynamically
    memcpy(directionalLightBufferMappedSources[currentImage],&directionalLightValue,sizeof(DirLightUniform));
}

void DefaultSceneDescriptorSets::UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity,const unsigned int currentImage) {
    DirLightUniform updatedValue = {
    .VLightDirection = glm::normalize(glm::vec3(mainCameraUniformValue.view * glm::vec4(direction,0.0f))),
    .DirLightColor = color,
    .DirLightIntensity = intensity};
    UpdateDirectionalLight(updatedValue,currentImage);
}



VkDescriptorSetLayout DefaultSceneDescriptorSets::GetLitDescriptorSetLayout() const {
    return litDescriptorSetLayout;
}

const VkDescriptorSet *DefaultSceneDescriptorSets::GetLitDescriptorSet(const uint32_t currentFrame) const{
    return &directionalLightDescriptorSets[currentFrame];
}

std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> DefaultSceneDescriptorSets::GetLitDescriptorSets() const {
    return mainCameraViewMatrixDescriptorSets;
}

void DefaultSceneDescriptorSets::UpdateCamera(const CameraUniform &updatedValue, unsigned int currentFrame) {
    mainCameraUniformValue = updatedValue;
    memcpy(mainCameraViewMatrixMappedSource[currentFrame],&mainCameraUniformValue,sizeof(CameraUniform));
}

VkDescriptorSetLayout DefaultSceneDescriptorSets::GetCameraDescriptorSetLayout() const {
    return mainCameraDescriptorSetLayout;
}

const VkDescriptorSet *DefaultSceneDescriptorSets::GetCameraDescriptorSet(uint32_t currentFrame) const {
    return &mainCameraViewMatrixDescriptorSets[currentFrame];
}

std::array<VkDescriptorSet, 2> DefaultSceneDescriptorSets::GetCameraDescriptorSets() const {
    return mainCameraViewMatrixDescriptorSets;
}

void DefaultSceneDescriptorSets::CreateGlobalPipelineLayout(VkDevice _device) {
    //mandatory for compatibility
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(Descriptors::UniformBufferObject)};
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &mainCameraDescriptorSetLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange,};

    //I could put the model push constant definition here, it would make sense
    if (vkCreatePipelineLayout(_device,&pipelineLayoutCreateInfo,nullptr, &GlobalMainPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create global pipeline layout");
    }
}

void DefaultSceneDescriptorSets::CreateLitPipelineLayout(VkDevice _device) {
    std::array<VkDescriptorSetLayout,2> LitSetLayouts = {mainCameraDescriptorSetLayout,litDescriptorSetLayout};
    //mandatory for compatibility
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(Descriptors::UniformBufferObject)};
    const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = static_cast<uint32_t>(LitSetLayouts.size()),
    .pSetLayouts = LitSetLayouts.data(),
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange,};
    if (vkCreatePipelineLayout(_device,&pipelineLayoutCreateInfo,nullptr,&LitPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create lit pipeline layout");
    }
}
