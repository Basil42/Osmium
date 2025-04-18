#version 450

layout(location = 0)in vec3 inViewDir;
layout(location = 1)in vec2 inUV;//might not need that with subpass input
//could probably wrap this in a set
layout(set = 1,binding = 1)uniform UBO{
    mat4 invProjMat;
    vec2 depthRange;//min max
};
layout(input_attachment_index = 0,set = 1, binding = 2)uniform subpassInput depthBuffer;//wondering how ok is it to read from all these draw calls
layout(input_attachment_index = 1,set = 1, binding = 3)uniform subpassInput normalAndSpreadBuffer;
layout(push_constant)uniform PointLight {
    layout(offset = 64)float radius;
    vec4 position;//view
    vec4 color;//alpha is intensity
} Light;


layout(location = 1)out vec3 outDiffuse;
layout(location = 2)out vec3 outSpecular;
//I might be able to just treat view position as 0 if I do everything in view space
//I don't see how to calculate light data from only normals and the spread factor, maybe I could use the reverse view matrix
void main() {
    //calculating viewPosition of the lit surface, it might indeed be better to just have a whole buffer
    float depth = subpassLoad(depthBuffer).r * 2.0 - 1.0;//depth texture might be an alpha only txture?
    vec4 clipPosition = vec4(inUV.xy * 2.0,depth,1.0);
    vec4 viewSpaceSurfacePosition = invProjMat * clipPosition;
    viewSpaceSurfacePosition = viewSpaceSurfacePosition/viewSpaceSurfacePosition.w;
//    float ndcZ = (2.0 * depth - depthRange.x - depthRange.y)/(depthRange.y - depthRange.x);
//    float viewZ = ProjMat[3][2] / ((ProjMat[2][3] * ndcZ) - ProjMat[2][2]);//check my proj matrix fullfill requirements
//    vec3 viewSpaceSurfacePosition = inViewDir * viewZ;

    vec4 normalAndSpread = subpassLoad(normalAndSpreadBuffer);
    //light calc
    vec3 lightDir = normalize(Light.position.xyz - viewSpaceSurfacePosition.xyz);
    float lambertian = max(dot(lightDir,normalAndSpread.xyz),0.0);
    float specular = 0;
    if(lambertian > 0.0f){
        vec3 halfVector = normalize(lightDir+inViewDir);
        float specAngle = max(dot(halfVector,normalAndSpread.xyz),0.0);
        specular = pow(specAngle,normalAndSpread.a);
    }


    float attenuationFactorB = 1.0 / (Light.radius * Light.radius * 0.01);
    float lightDistance = (viewSpaceSurfacePosition.xyz - Light.position.xyz).length();
    float attenuation = 1.0 / (1.0 + attenuationFactorB * lightDistance * lightDistance);
    //additively blended
    outDiffuse = lambertian * Light.color.rgb * Light.color.a * attenuation;
    outSpecular = specular * Light.color.rgb * Light.color.a * attenuation;
}