#version 450

layout(set = 0, binding = 0)uniform CameraMatricesObject{
    mat4 view;
    mat4 proj;
} VP;
layout(set = 1, binding = 0)uniform clipSpaceInfo{
    vec2 ScreenSize;
    vec2 halfSizeNearPlane;
};

layout(location = 0)out vec3 eyeDir;
layout(location = 1)out vec4 clipPosition;

void main() {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2,gl_VertexIndex & 2);
    clipPosition = vec4(uv * 2.0f + -1.0f,0.0f,1.0f);
    gl_Position = clipPosition;
    eyeDir = -vec3((2.0 * halfSizeNearPlane * uv) - halfSizeNearPlane, -1.0);
}