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
#include "ErrorChecking.h"
#include "UniformBufferObject.h"

VkPipelineLayout DefaultSceneDescriptorSets::GetCameraPipelineLayout() const {
    return GlobalOpaquePipelineLayout;
}

VkPipelineLayout DefaultSceneDescriptorSets::GetLitPipelineLayout() const {
    return DirLightPassPipelineLayout;
}

VkPipelineLayout DefaultSceneDescriptorSets::GetPointLightPipelineLayout() const {
    return PointLightPassPipelineLayout;
}

void DefaultSceneDescriptorSets::CreateDefaultDescriptorPool(const VkDevice _device) {
    std::array<VkDescriptorPoolSize, BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT> sizes{};

//camera,dirlight and point light
    sizes[0] = VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    MAX_FRAMES_IN_FLIGHT * 3);


    VkDescriptorPoolCreateInfo descriptorPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *3),
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


    const VkDescriptorSetLayoutCreateInfo  dirLightDescriptorLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &directionalLightLayoutBinding,
    };

    if (vkCreateDescriptorSetLayout(_device, &dirLightDescriptorLayoutCreateInfo, nullptr, &DirLightUniform.DescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buitl in descriptor set layout!");
    }
    //view matrix, I could also reuse the struct, but thsi is clearer
    constexpr VkDescriptorSetLayoutBinding viewMatrixLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr};

    const VkDescriptorSetLayoutCreateInfo viewMatrixDescriptorLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &viewMatrixLayoutBinding,};

    if (vkCreateDescriptorSetLayout(_device,&viewMatrixDescriptorLayoutCreateInfo,nullptr,&CameraUniform.DescriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create view matrix built in descriptor set layout!");
    }
    constexpr VkDescriptorSetLayoutBinding ClipSpaceInfoLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};


    constexpr VkDescriptorSetLayoutBinding DepthBufferBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};
    constexpr VkDescriptorSetLayoutBinding normalSpreadBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};
    //projmat and depthRange
    constexpr VkDescriptorSetLayoutBinding ReconstructionDataBinding = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    std::array pointLightBidings{ClipSpaceInfoLayoutBinding,DepthBufferBinding,normalSpreadBinding,ReconstructionDataBinding};
    const VkDescriptorSetLayoutCreateInfo pointLightDescriptorSetLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 4,
        .pBindings = pointLightBidings.data()};
    check_vk_result(vkCreateDescriptorSetLayout(device,&pointLightDescriptorSetLayoutInfo,nullptr,&PointLightUniform.DescriptorLayout));
}

void DefaultSceneDescriptorSets::CreateDescriptorSets(VkDevice device, VmaAllocator allocator,
                                                      const OsmiumGLDynamicInstance &GLInstance, DefaultSceneDescriptorSets::UniformData &uniform_data, std::size_t
                                                      uniformSize) {
   std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> descriptorSetLayouts = {};
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        descriptorSetLayouts[i] = uniform_data.DescriptorLayout;
    }
    //sets
    VkDescriptorSetAllocateInfo dirLightDescriptorSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = descriptorSetLayouts.data()
    };
    if (vkAllocateDescriptorSets(device,&dirLightDescriptorSetAllocateInfo,uniform_data.DescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for directional light!");
    }
    //buffers
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        const VkDeviceSize bufferSize = uniformSize;

        GLInstance.createBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                uniform_data.Buffers[i],
                                uniform_data.BuffersMemory[i],
                                VMA_MEMORY_USAGE_AUTO,
                                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        auto result = vmaMapMemory(allocator,uniform_data.BuffersMemory[i],&uniform_data.mappedSources[i]);
        assert(result == VK_SUCCESS);

    }
    //writes, maybe object should do their own ?
     for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
         VkDescriptorBufferInfo descriptorBufferInfo = {
             .buffer = uniform_data.Buffers[i],
             .offset = 0,
             .range = uniformSize};

         std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
         descriptorWrites[0] = {
             .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
             .dstSet = uniform_data.DescriptorSets[i],
             .dstBinding = 0,
             .dstArrayElement = 0,
             .descriptorCount = 1,
             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
             .pImageInfo = nullptr,
             .pBufferInfo = &descriptorBufferInfo,
             .pTexelBufferView = nullptr};
         vkUpdateDescriptorSets(device,static_cast<uint32_t>(descriptorWrites.size()),descriptorWrites.data(),0, nullptr);
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
#ifndef DYNAMIC_RENDERING
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
                         sizeof(DirLightUniformValue));



    //camera
    CreateDescriptorSets(_device, allocator, GLInstance,
                         mainCameraDescriptorSetLayout,
                         mainCameraViewMatrixDescriptorSets,
                         mainCameraViewMatrixUniformBuffers,
                         mainCameraViewMatrixAllocations,
                         mainCameraViewMatrixMappedSource,
                         sizeof(CameraUniformValue));

    CreateGlobalPipelineLayout(_device);
    CreateLitPipelineLayout(_device);
}
#endif
DefaultSceneDescriptorSets::DefaultSceneDescriptorSets(VkDevice device, VmaAllocator allocator,
    OsmiumGLDynamicInstance &GLinstance) {
    this->device = device;
    CreateDefaultDescriptorPool(device);//TODO add affordances for point lights
    CreateDefaultDescriptorLayouts(device);//TODO move point light layout creaation here
    Allocator = allocator;
    //might need to initialize all these value to reasonable values
    //dir light
    CreateDescriptorSets(device,allocator, GLinstance,DirLightUniform, sizeof(DirLightUniformValue));
    //point lights
    CreateDescriptorSets(device,allocator, GLinstance,PointLightUniform, sizeof(PointLightUniformValue));
    //Camera
    CreateDescriptorSets(device,allocator,GLinstance,CameraUniform, sizeof(CameraUniformValue));
    //these might not really work anymore, may be split into global pipeline per pass
    CreateGlobalOpaquePipelineLayout(device);
    CreateDirectionalLightPassPipelineLayout(device);
    CreatePointLightPassPipelineLayout(device);
}

DefaultSceneDescriptorSets::~DefaultSceneDescriptorSets() {
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        vmaUnmapMemory(Allocator,DirLightUniform.BuffersMemory[i]);
        vmaDestroyBuffer(Allocator,DirLightUniform.Buffers[i],DirLightUniform.BuffersMemory[i]);
        vmaUnmapMemory(Allocator,CameraUniform.BuffersMemory[i]);
        vmaDestroyBuffer(Allocator,CameraUniform.Buffers[i],CameraUniform.BuffersMemory[i]);
        vmaUnmapMemory(Allocator,PointLightUniform.BuffersMemory[i]);
        vmaDestroyBuffer(Allocator,PointLightUniform.Buffers[i],PointLightUniform.BuffersMemory[i]);
    }
    vkDestroyDescriptorPool(device,descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device,DirLightUniform.DescriptorLayout,nullptr);
    vkDestroyDescriptorSetLayout(device,CameraUniform.DescriptorLayout,nullptr);
    vkDestroyDescriptorSetLayout(device,PointLightUniform.DescriptorLayout,nullptr);


    vkDestroyPipelineLayout(device,GlobalOpaquePipelineLayout,nullptr);
    vkDestroyPipelineLayout(device,DirLightPassPipelineLayout,nullptr);
    vkDestroyPipelineLayout(device,PointLightPassPipelineLayout,nullptr);

}

void DefaultSceneDescriptorSets::UpdateDirectionalLight(const DirLightUniformValue &updatedValue, const unsigned int currentImage) {
    directionalLightValue = updatedValue;
    //I'll update all frames manually for now, but I feel like this will cause some problems if I want to use it dynamically
    memcpy(DirLightUniform.mappedSources[currentImage],&directionalLightValue,sizeof(DirLightUniformValue));
}

void DefaultSceneDescriptorSets::UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity,const unsigned int currentImage) {
    DirLightUniformValue updatedValue = {
    .VLightDirection = glm::normalize(glm::vec3(mainCameraUniformValue.view * glm::vec4(direction,0.0f))),
    .DirLightColor = color,
    .DirLightIntensity = intensity};
    UpdateDirectionalLight(updatedValue,currentImage);
}

void DefaultSceneDescriptorSets::UpdatePointLightUniform(const PointLightUniformValue& newValue,const unsigned int currentImage) {
    pointLightValue = newValue;
    memcpy(PointLightUniform.mappedSources[currentImage],&pointLightValue,sizeof(PointLightUniformValue));

}




VkDescriptorSetLayout DefaultSceneDescriptorSets::GetLitDescriptorSetLayout() const {
    return DirLightUniform.DescriptorLayout;
}

const VkDescriptorSet *DefaultSceneDescriptorSets::GetLitDescriptorSet(const uint32_t currentFrame) const{
    return &DirLightUniform.DescriptorSets[currentFrame];
}

const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& DefaultSceneDescriptorSets::GetLitDescriptorSets() const {
    return DirLightUniform.DescriptorSets;
}

VkDescriptorSetLayout &DefaultSceneDescriptorSets::GetPointLightSetLayout() {
    return PointLightUniform.DescriptorLayout;
}

const VkDescriptorSet * DefaultSceneDescriptorSets::GetPointLightSet(uint32_t currentFrame) const {
    return &PointLightUniform.DescriptorSets[currentFrame];
}

void DefaultSceneDescriptorSets::UpdateCamera(const CameraUniformValue &updatedValue, unsigned int currentFrame) {
    mainCameraUniformValue = updatedValue;
    memcpy(CameraUniform.mappedSources[currentFrame],&mainCameraUniformValue,sizeof(CameraUniformValue));
}

VkDescriptorSetLayout DefaultSceneDescriptorSets::GetCameraDescriptorSetLayout() const {
    return CameraUniform.DescriptorLayout;
}

const VkDescriptorSet& DefaultSceneDescriptorSets::GetCameraDescriptorSet(uint32_t currentFrame) const {
    return CameraUniform.DescriptorSets[currentFrame];
}

const std::array<VkDescriptorSet, 2>& DefaultSceneDescriptorSets::GetCameraDescriptorSets() const {
    return CameraUniform.DescriptorSets;
}

void DefaultSceneDescriptorSets::CreateGlobalOpaquePipelineLayout(VkDevice _device) {
    //mandatory for compatibility
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(Descriptors::UniformBufferObject)};
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &CameraUniform.DescriptorLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange,};

    if (vkCreatePipelineLayout(_device,&pipelineLayoutCreateInfo,nullptr, &GlobalOpaquePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create global pipeline layout");
    }
}

void DefaultSceneDescriptorSets::CreateDirectionalLightPassPipelineLayout(VkDevice _device) {

    std::array<VkDescriptorSetLayout,2> LitSetLayouts = {CameraUniform.DescriptorLayout,DirLightUniform.DescriptorLayout};
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
    if (vkCreatePipelineLayout(_device,&pipelineLayoutCreateInfo,nullptr,&DirLightPassPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create lit pipeline layout");
    }
}
void DefaultSceneDescriptorSets::CreatePointLightPassPipelineLayout(VkDevice _device) {
    std::array SetLayouts = {CameraUniform.DescriptorLayout,PointLightUniform.DescriptorLayout};
    std::array<VkPushConstantRange,2> pushConstantRanges;
    pushConstantRanges[0]= {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(PointLightPushConstants::vertConstant) + sizeof(float)};
    pushConstantRanges[1]= {
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = offsetof(PointLightPushConstants,PointLightPushConstants::radius),
        .size = sizeof(PointLightPushConstants::fragConstant) + 16
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    std::array pointLightDescriptorLayouts{CameraUniform.DescriptorLayout,PointLightUniform.DescriptorLayout};
    pipelineLayoutCreateInfo.setLayoutCount = 2;
    pipelineLayoutCreateInfo.pSetLayouts = pointLightDescriptorLayouts.data();//might have to have severa to get the camera set
    pipelineLayoutCreateInfo.pushConstantRangeCount = 2;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
    check_vk_result(vkCreatePipelineLayout(device,&pipelineLayoutCreateInfo,nullptr,&PointLightPassPipelineLayout));

}
