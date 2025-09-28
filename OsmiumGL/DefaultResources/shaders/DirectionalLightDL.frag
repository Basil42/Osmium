#version 450

layout(location = 0)in vec3 inViewDir;
layout(location = 1)in vec4 inClipPos;

layout(set = 1,binding =1) uniform UBO{
    layout(offset = 16)mat4 invProjMat;
    vec2 depthRange;//might be more useful that in the point light, might still need ot be removed
};
layout(input_attachment_index = 0, set = 1, binding = 2)uniform subpassInput depthBuffer;
layout(input_attachment_index = 1, set = 1, binding = 3)uniform subpassInput normalAndSpreadBuffer;

layout (push_constant) uniform DirectionalLightInfo {
    vec4 color;
    vec3 Direction;//should be view coordinates
};

layout (location = 0) out vec4 outDiffuse;
layout (location = 1) out vec4 outSpecular;

layout(constant_id = 0)const float MaxSpecularPower = 32.0f;

void main() {
    float depth = subpassLoad(depthBuffer).r;
    vec4 ClipPosWithDepth = vec4(inClipPos.xy,depth,inClipPos.w);
    vec4 viewSpaceSurfacePosition = invProjMat * ClipPosWithDepth;
    viewSpaceSurfacePosition = viewSpaceSurfacePosition/viewSpaceSurfacePosition.w;

    vec4 normalAndSpread = subpassLoad(normalAndSpreadBuffer);
    //light calc
    float lambertian = max(dot(Direction,normalAndSpread.xyz),0.0f);
    float specular = 0.0f;
    if(lambertian > 0.0f){
        vec3 halfVector = normalize(Direction+inViewDir);
        float specAngle = max(dot(halfVector,normalAndSpread.xyz),0.0f);
        specular = pow(specAngle,normalAndSpread.a*MaxSpecularPower);
        specular = ((MaxSpecularPower + 8)/8) * specular;
    }

    //vec3 IntensityColor = color.rgb * color.a;
    outDiffuse = lambertian * color;
    outSpecular = specular * lambertian * color;
}