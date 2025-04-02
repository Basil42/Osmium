#version 450
layout(location = 0)in vec2 TexCoord;
layout(location = 1)in vec3 viewPos;

layout(set = 1, binding = 0)uniform sampler2D alebedoMap;
layout(set = 1, binding = 1)uniform sampler2D specularMap;
layout(set = 1, binding = 2)uniform AmbientLightInfo{
    vec4 AmbientLight;//could use alpha as an intensity modifier I guess
};

layout(input_attachment_index = 0,set = 1, binding = 3)uniform subpassInput NormalSpreadBuffer;
layout(input_attachment_index = 1,set = 1, binding = 4)uniform subpassInput DiffuseBuffer;
layout(input_attachment_index = 2,set = 1, binding = 5)uniform subpassInput SpecularBuffer;


layout(location = 0)out vec4 outColor;
void main() {
    vec3 viewDir = normalize(-viewPos);
    vec4 albedo = texture(alebedoMap,TexCoord);
    vec4 specularColor = texture(specularMap,TexCoord);
    specularColor = specularColor + (1-specularColor)*(1-max(dot(subpassLoad(NormalSpreadBuffer).xyz,viewDir),0.0));
    outColor = albedo * (AmbientLight + subpassLoad(DiffuseBuffer)) + subpassLoad(SpecularBuffer) * specularColor;
}