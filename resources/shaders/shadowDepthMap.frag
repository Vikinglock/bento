#version 450

layout(location = 0) out float depth;
layout(location = 1) out vec4 a;

layout(location = 0) in vec3 fragPos;

layout(set = 0, binding = 0) uniform sampler2D tex;

//layout(set = 0, binding = 7) uniform Uniforms {};

void main() {
    depth = fragPos.z;
    a = vec4(fragPos,1.0);
}