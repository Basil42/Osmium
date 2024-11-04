//
// Created by nicolas.gerard on 2024-11-04.
//

#ifndef SHADERUTILITIES_H
#define SHADERUTILITIES_H
#include <fstream>
#include <vector>

namespace ShaderUtils{
    static std::vector<char> readfile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if(!file.is_open()) {
            throw std::runtime_error("Failed to open file " + filename);
        }
        size_t filesize = (size_t)file.tellg();
        std::vector<char> buffer(filesize);
        file.seekg(0);
        file.read(buffer.data(), filesize);
        file.close();
        return buffer;
    }
    inline VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
        return shaderModule;
    }
}
#endif //SHADERUTILITIES_H
