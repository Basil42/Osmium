#version 450
layout(set = 1,binding = 0)uniform sampler2D smoothnessMap;

layout(location = 0)in vec3 inNormal;
layout(location = 1)in vec2 inTexcoordinate;

layout(location = 0)out vec4 Normal_smoothness;
//layout(location = 1)out vec4 Diffuse;
//layout(location = 2)out vec4 Specular;
//layout(location = 3)out vec4 finalColor;


void main() {
    Normal_smoothness = vec4(normalize(inNormal),texture(smoothnessMap,inTexcoordinate).r);
//    Diffuse = vec4(0.0);
//    Specular = vec4(0.0);
      //finalColor = vec4(0.0);

}