#version 450
layout(push_constant,std430)uniform UniformBufferObject{
    mat4 model;
} ubo;
layout(set = 0, binding = 0)uniform CameraMatricesObject {
    mat4 view;
    mat4 proj;
} VP;

//vertex attributes
layout(location = 0)in vec3 inPosition;
layout(location = 1)in vec2 inTexcoordinate;
layout(location = 2)in vec3 inNormal;

layout(location = 0)out vec3 Normal;
layout(location = 1)out vec2 Texcoordinate;


void main() {
    gl_Position = VP.proj * VP.view * ubo.model * vec4(inPosition,1.0);
    mat3 normalMat = transpose(inverse(mat3(mat3(ubo.model))));
    Normal = normalize(normalMat * inNormal);//not sure it is that useful to normilize here

    Texcoordinate = inTexcoordinate;
}