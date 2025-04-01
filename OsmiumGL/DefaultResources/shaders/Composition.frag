#version 450
//this is for a fully defered renderer
layout (input_attachment_index = 0, binding = 0) uniform subpassInput positionDepthAttachment;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;
layout (input_attachment_index = 3, binding = 3) uniform subpassInput specularAttachment;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;



struct DirectionalLight{
    vec3 Direction;//probably world in this case
    vec4 Color;//alpha is intensity
    vec4 ambient;//sneaking it here
};
layout(binding = 4)uniform DirectionalLight dirLight;
struct PointLight {
    vec4 position;
    vec4 color;//alpha is intensity
    float radius;
};
//not sure how to mix attachement and uniform sets
layout(std140, binding = 5) readonly buffer PointLightsBuffer{
    PointLight pointLights[];
};

struct SpotLight{
    vec4 position;
    vec4 color;//alpha is intensity
    vec3 Direction;
    vec2 angles;
};
layout(binding = 6)readonly buffer SpotLightsBuffer {
    SpotLight spotlights[];
};
layout(binding = 7)uniform vec3 viewPos;//worlpos of the camera

void main() {
    vec3 fragPos = subpassLoad(positionDepthAttachment).rgb;
    vec3 normal = normalize(subpassLoad(normalAttachment).rgb);//check this normal is in the same space as my blinn phong one
    vec4 albedo = subpassLoad(albedoAttachment);
    vec4 specularity = subpassLoad(specularAttachment);
    //do I need extra attachement for specular/ ?
    vec3 fragColor = albedo.rgb * dirLight.ambient;



    //dir light
    {
        //vec3 lightDir =
        //float lambertian = max(dot())
    }
//    vec3 DirLightViewDir =  dirlight.position
//    float lambertian = max(dot())
    //point lights
    for(int i = 0; i < pointLights.length();i++){
        PointLight light = pointLights[i];
        vec3 lightDir = light.position.xyz - fragPos;
        float lambertian = max(dot(lightDir,normal),0.0);
        float specular = 0;
        if(lambertian > 0.0f){
            vec3 eyeDir = normalize(viewPos-fragPos);
            vec3 halfVector = normalize(lightDir+eyeDir);
            float specAngle = max(dot(halfDir,normal),0.0);
            specular = pow(specAngle,specularity.a);

        }
        fragColor += (light.color.rgb * light.color.a) * (albedo * lambertian + specularity.rgb * specular);

    }

}