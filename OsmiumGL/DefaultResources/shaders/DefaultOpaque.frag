#version 450
layout(binding = 0)uniform sampler2D AlbedoMap;

layout(location = 0)in OpaqueFragmentInput{
    vec3 inWorldPos;
    vec3 inNormal;
    vec2 TexCoord;
};

layout(location = 0) out vec4 outColor;//seems strange to not use a vec3 format here
layout(location = 1) out vec4 outPositionDepth;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outAlbedo;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;//not sure about these values

//derived from vulkan samples
float LinearDepth(float depth){
    float z = depth *2.0f -1.0f;//remap from -1 to 1
    return (2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z *(FAR_PLANE -NEAR_PLANE));
}

void main() {
    outPositionDepth = vec4(inWorldPos,1.0);

    vec3 N = normalize(inNormal);
    //yflip
    //N.y = -N.y
    outNormal = vec4(N,1.0);

    outAlbedo = texture(AlbedoMap,TexCoord);//i could pack a specular intensity in this

    outPositionDepth.a = LinearDepth(gl_FragCoord.z);

    //writing white for now as a test
    outColor = vec4(1.0);//not writing to the color attachement is undefined behavior, which is a bit wild
}