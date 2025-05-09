#version 450

layout(push_constant,std430)uniform UniformBufferObject{
    mat4 model;
} ubo;
layout(set = 0, binding = 0)uniform CameraMatricesObject {
    mat4 view;
    mat4 proj;
} VP;

layout(location = 0)in vec3 inPosition;
layout(location = 1)in vec2 inTexCoordinates;
//don't need normals anymore
//layout(location = 2)in vec3 inNormal;

layout(location = 0)out vec2 TexCoordinates;
layout(location = 1)out vec3 outViewPosition;

void main() {
    outViewPosition = (VP.view * ubo.model * vec4(inPosition,1.0)).xyz;
    gl_Position = VP.proj * vec4(outViewPosition,1.0);
    TexCoordinates = inTexCoordinates;
}