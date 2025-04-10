//
// Created by nicolas.gerard on 2025-04-10.
//

#include "ParsedMaterial.h"

#include <cassert>
#include <ShaderUtilities.h>

#include "spirv_reflect.h"

ParsedMaterial::ParsedMaterial(std::span<std::filesystem::path> shaders) {
    assert(shaders.size() % 2 == 0);//checking that shaders are in pairs
    uint8_t shaderIndex = 0;
    while (shaderIndex < shaders.size()) {
        const auto& shaderPath = shaders[shaderIndex++];
        std::vector<char> vertexCode = ShaderUtils::readfile(shaderPath.string());
        SpvReflectShaderModule vertexModule;
        SpvReflectResult result = spvReflectCreateShaderModule(vertexCode.size(),vertexCode.data(), &vertexModule);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);


    }

}
