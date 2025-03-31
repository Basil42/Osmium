#version 450
layout(location = 0)in vec4 inPosition;//using vec3 doesn't save any memory because of aligment
layout(location = 1)in vec2 inTexCoordinates;
layout(location = 2)in vec3 inNormal;


layout(push_constant)uniform UniformBufferObject {
    mat4 model;
    mat4 normal;//could calculate this on the spot, might be wiser
} ubo;

layout(set = 0,binding = 0)uniform CameraMatricesObject {
    mat4 view;
    mat4 proj;
}VP;

layout(location = 0)out OpaqueFragInput {
    vec3 outWorldPos;
    vec3 outNormal;
    vec2 TexCoord0;

} ;


void main() {
    gl_position = VP.proj * VP.view * ubo.model * inPosition;

    outworldpos = vec3(ubo.model * inPosition);
    //outWorldPos.y = -outWorldPos.y;//vulkan coordinates, but I can account for this CPU side

    mat3 normalMat = transpose(inverse(mat3(mat3(ubo.model))));//probably faster to do it here
    outnormal = normalize(normalMat * inNormal);

    TexCoord0 = inTexCoordinates;
}