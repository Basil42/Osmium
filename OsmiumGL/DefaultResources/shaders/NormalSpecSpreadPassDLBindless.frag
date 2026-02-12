#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0,binding = 0)uniform sampler2D Textures[];
layout(push_constant)uniform NormSpecData{
    layout(offset = 64) uint SmoothnessMapIndex;
};

layout(location = 0)in vec3 inNormal;
layout(location = 1)in vec2 inTexcoordinate;

layout(location = 0)out vec4 Normal_smoothness;
layout(location = 1)out vec4 Diffuse;
layout(location = 2)out vec4 Specular;
layout(location = 3)out vec4 finalColor;


void main() {
    Normal_smoothness = vec4(normalize(inNormal),texture(Textures[SmoothnessMapIndex],inTexcoordinate).r);
    Diffuse = vec4(0.0);
    Specular = vec4(0.0);
    finalColor = vec4(0.0);

}