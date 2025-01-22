#version 450

layout(binding = 1)uniform DirLightBlock//3 locations
{
    vec3 VLightDirection;
    vec3 DirLightColor;
    float DirLightIntensity;
};
layout(binding = 2)uniform sampler2D texSampler;
const vec3 AmbLightColor = vec3(0.1,0.1,0.1);
//should be handled by the sampler
//layout(binding = 4)uniform screenGamma

layout(location = 0)in vec4 VertexColor;
layout(location = 1)in BlingPhongFragInput{//3locations
    vec2 TexCoord0;
    vec3 VPosition;//view position, before projection
    vec3 VNormal;//interp, needs normailizing
    vec3 DiffuseColor;
    vec3 SpecColor;
    float Shininess;//Smoothness maybe ?
};

layout(location = 0)out vec4 outColor;

void main() {
    vec3 normal = normalize(VNormal);
    //here you'd compute distance for a non infinitely distance light source

    float lambertian = max(dot(VLightDirection,normal),0.0);
    float specular = 0;

    if(lambertian > 0.0f){//there's probably a way of getting rid of this one
        vec3 viewDir = normalize(-VPosition);

        vec3 halfDir = normalize(VLightDirection + viewDir);
        float specAngle = max(dot(halfDir, normal),0.0);
        specular = pow(specAngle,Shininess);
    }
    vec3 colorLinear = AmbLightColor +
    (DirLightColor * DirLightIntensity)*
    (DiffuseColor * lambertian +
    SpecColor * specular) ;
    //maybe linearize here
    outColor = vec4(colorLinear,1.0);
}