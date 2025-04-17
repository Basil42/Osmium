#version 450



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


void main() {

    vec4 clipPos = VP.proj * VP.view * pld.model * vec4(normalize(pld.model[3].xyz) * pld.radius,1.0);//error isn't real, again
    gl_Position = clipPos;//distribute along a sphere
    uv = clipPos.xy /ScreenSize;
    eyeDir = vec3((2.0 * halfSizeNearPlane * uv) - halfSizeNearPlane, -1.0);


}