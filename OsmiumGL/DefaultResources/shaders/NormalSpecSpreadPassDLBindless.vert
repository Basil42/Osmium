#version 450
layout(push_constant,std430)uniform UniformBufferObject{//TODO split the push constant in it's vert and frag part, the frag part should hold index to the smoothness map
    mat4 model;
} ubo;
//not used here but it would be valid on all shaders here and would always be this set
//layout(set = 0, binding = 0)uniform Sampler2D inTexture[];
layout(set = 1, binding = 0)uniform CameraMatricesObject {
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
    //Normal = normalize(normalMat * inNormal);//not sure it is that useful to normilize here
    vec4 viewNormal = VP.view * vec4(normalMat * inNormal,0);
    Normal = viewNormal.xyz;
    Texcoordinate = inTexcoordinate;
}