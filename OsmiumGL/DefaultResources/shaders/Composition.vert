#version 450

layout(location = 0) out vec2 outUV;

void main() {
    //full screen quad
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_position = vec4(outUV * 2.0f -1.0f,0.0f,1.0f);
}