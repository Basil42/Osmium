#version 450

layout(location = 0)in vec3 inViewDir;
layout(location = 1)in vec2 inUV;//might not need that with subpass input
layout(location = 2)in vec4 viewLightCenter;
//could probably wrap this in a set
layout(set = 1,binding = 1)uniform UBO{
    layout(offset = 16)mat4 invProjMat;
    vec2 depthRange;//remove
};
layout(input_attachment_index = 0,set = 1, binding = 2)uniform subpassInput depthBuffer;//wondering how ok is it to read from all these draw calls
layout(input_attachment_index = 1,set = 1, binding = 3)uniform subpassInput normalAndSpreadBuffer;
layout(push_constant)uniform PointLight {
    layout(offset = 64)float radius;
    vec4 color;//alpha is intensity
} Light;


layout(location = 1)out vec3 outDiffuse;
layout(location = 2)out vec3 outSpecular;

layout(constant_id = 0)const float MaxSpecularPower = 32.0f;
//I might be able to just treat view position as 0 if I do everything in view space
//I don't see how to calculate light data from only normals and the spread factor, maybe I could use the reverse view matrix
void main() {
    //calculating viewPosition of the lit surface, it might indeed be better to just have a whole buffer
    float depth = subpassLoad(depthBuffer).r;// * 2.0 - 1.0;//depth texture might be an alpha only txture?
    vec4 clipPosition = vec4(inUV.xy * 2.0 -1.0,depth,1.0);
    vec4 viewSpaceSurfacePosition = invProjMat * clipPosition;
    viewSpaceSurfacePosition = viewSpaceSurfacePosition/viewSpaceSurfacePosition.w;

    vec3 viewLightCenterLinear = viewLightCenter.xyz / viewLightCenter.w;
    vec4 normalAndSpread = subpassLoad(normalAndSpreadBuffer);
    //light calc
    vec3 lightDir = normalize(viewLightCenterLinear - viewSpaceSurfacePosition.xyz);
    float lambertian = max(dot(lightDir,normalAndSpread.xyz),0.0);
    float specular = 0;
    if(lambertian > 0.0f){
        vec3 halfVector = normalize(lightDir+inViewDir);
        float specAngle = max(dot(halfVector,normalAndSpread.xyz),0.0);
        specular = pow(specAngle,normalAndSpread.a*MaxSpecularPower);
        specular = ((MaxSpecularPower + 8)/8) * specular;
    }



    float attenuationFactorB = 1.0 / (Light.radius * Light.radius * 0.01);
    float lightDistance = (viewSpaceSurfacePosition.xyz - viewLightCenterLinear).length();
    float attenuation = 1.0 / (1.0 + attenuationFactorB * lightDistance * lightDistance);
    //additively blended
    vec3 IntensityColor = Light.color.rgb * Light.color.a * attenuation;
    outDiffuse = lambertian * IntensityColor;
    outSpecular = specular * lambertian * IntensityColor;
}