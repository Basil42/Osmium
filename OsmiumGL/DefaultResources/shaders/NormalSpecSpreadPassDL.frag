#version 450
layout(set = 1,binding = 0)uniform sampler2D smoothnessMap;//seems a bit overkill just for this, but I'll probably be able to sqeeze more data in there

layout(location = 0)in vec3 inNormal;
layout(location = 1)in vec2 inTexcoordinate;

layout(location = 0)out vec4 Normal_smoothness;//hoping to only need a single attachement here
//might need a ouColor field to write into the attachement, which I find a bit suspicious

void main() {
    Normal_smoothness = vec4(normalize(inNormal),texture(smoothnessMap,inTexcoordinate).r);
    //write to color attachment if needed
}