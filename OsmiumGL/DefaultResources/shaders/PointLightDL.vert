#version 450

layout(location = 0)vec4 inPosition;

layout(push_constant)uniform PointLightData{
    mat4 model;
    float radius;//might need a more complicated block than this
} pld;




void main() {
  gl_position =   model * (normalize(inPosition) * radius);//should distribute all vertices on a sphere of the correct radius
}