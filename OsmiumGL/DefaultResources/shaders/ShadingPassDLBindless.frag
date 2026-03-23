#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0,binding = 0)uniform sampler2D Textures[];

//texture descriptor
layout(set = 1, binding = 1)uniform AmbientLightInfo{
    vec4 AmbientLight;//could use alpha as an intensity modifier I guess
};

layout(input_attachment_index = 0,set = 1, binding = 2)uniform subpassInput NormalSpreadBuffer;// i might recalculate these instead
layout(input_attachment_index = 1,set = 1, binding = 3)uniform subpassInput DiffuseBuffer;
layout(input_attachment_index = 2,set = 1, binding = 4)uniform subpassInput SpecularBuffer;

layout(push_constant)uniform shadingData{
    layout(offset = 68)uint albedoMapIndex;//this offset mus tinclude the light pass data between the model and the rendering data
    uint specularMapIndex;
};

layout(location = 0)in vec2 TexCoord;
layout(location = 1)in vec3 viewPos;

layout(location = 0)out vec4 outColor;
void main() {
    vec3 viewDir = normalize(-viewPos);
    vec4 albedo = texture(Textures[albedoMapIndex],TexCoord);
    vec4 specularColor = texture(Textures[specularMapIndex],TexCoord);
    specularColor = specularColor + (1-specularColor)*pow((1-max(dot(subpassLoad(NormalSpreadBuffer).xyz,viewDir),0.0)),5.0);
    outColor = albedo * (AmbientLight + subpassLoad(DiffuseBuffer)) + subpassLoad(SpecularBuffer) * specularColor;
}