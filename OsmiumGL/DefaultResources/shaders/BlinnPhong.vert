#version 450

layout(push_constant, std430)uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
} ubo;

layout(location = 0)in vec3 inPosition;
layout(location = 1)in vec3 inColor;
layout(location = 2)in vec2 inTexCoordinates;
layout(location = 3)in vec3 inNormal;

layout(location = 0)out vec4 VertexColor;
layout(location = 1)out BlingPhongFragInput{
    vec2 TexCoord0;
    vec3 VPosition;//view position, before projection
    vec3 VNormal;//interp, needs normailizing
    vec3 DiffuseColor;
    vec3 SpecColor;
    float Shininess;//Smoothness maybe ?
};

void main() {
    vec4 vertPos4 = ubo.view * ubo.model * vec4(inPosition,1.0);
    VPosition = vec3(vertPos4) / vertPos4.w;
    TexCoord0 = inTexCoordinates;
    vec4 VNormalHomogeneous = ubo.view * ubo.normal * vec4(inNormal,1.0);
    VNormal = vec3(VNormalHomogeneous)/VNormalHomogeneous.w;
    VertexColor = vec4(inColor,1.0f);
    DiffuseColor = inColor;
    SpecColor = inColor;
    Shininess = 0.4f;
}