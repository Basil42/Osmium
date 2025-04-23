#version 450


layout(location = 0)in vec3 inPosition;
layout(push_constant)uniform PointLightData{
    mat4 model;
    float radius;//might need a more complicated block than this
} pld;
layout(set = 0, binding = 0)uniform CameraMatricesObject {
    mat4 view;
    mat4 proj;
} VP;
layout(set = 1,binding = 0)uniform clipSpaceInfo{
    vec2 ScreenSize;
    vec2 halfSizeNearPlane;
};


layout(location = 0)out vec3 eyeDir;
layout(location = 1)out vec2 uv;
layout(location = 2)out vec4 viewCenter;

void main() {
    vec4 adjustedPosition = vec4(normalize(inPosition.xyz) * pld.radius,1.0);
    viewCenter = (VP.view * pld.model[3]);
    vec4 clipPos = VP.proj * VP.view * pld.model * adjustedPosition ;//error isn't real, again
    gl_Position = clipPos;//distribute along a sphere
    uv = ((clipPos.xy)/clipPos.w) * 0.5 + 0.5;
    eyeDir = -vec3((2.0 * halfSizeNearPlane * uv) - halfSizeNearPlane, -1.0);


}