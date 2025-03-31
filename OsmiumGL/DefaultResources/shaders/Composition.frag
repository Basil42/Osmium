#version 450
//this is for a fully defered renderer
layout (input_attachment_index = 0, binding = 0) uniform subpassInput positionDepthAttachment;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;



struct DirectionalLight{
    vec3 Direction;//probably world in this case
    vec4 Color;//alpha is intensity
    vec4 ambient;//sneaking it here
};
layout(binding = 3)uniform DirectionalLight dirLight;
struct PointLight {
    vec4 position;
    vec4 color;//alpha is intensity
    float radius;
};
//not sure how to mix attachement and uniform sets
layout(std140, binding = 4) readonly buffer PointLightsBuffer;

struct SpotLight{
    vec4 position;
    vec4 color;//alpha is intensity
    vec3 Direction;
    vec2 angles;
};
layout(binding = 5)readonly buffer SpotLightsBuffer;
layout(binding = 6)uniform vec3 viewPos;

void main() {
    vec3 fragPos = subpassLoad(positionDepthAttachment).rgb;
    vec3 normal = subpassLoad(normalAttachment).rgb;
    vec4 albedo = subpassLoad(albedoAttachment);
    //do I need extra attachement for specular/ ?
    vec3 fragColor = albedo.rgb * dirLight.ambient;

    //dir light
    //vec3 DirLightViewDir =  dirlight.position
    //float lambertian = max(dot())
}