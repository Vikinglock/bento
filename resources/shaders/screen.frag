#version 450

layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;
layout(location = 0) in vec2 fUv;

layout(set = 0, binding = 0) uniform sampler2D diftex;
layout(set = 0, binding = 1) uniform sampler2D nortex;
layout(set = 0, binding = 2) uniform sampler2D uvtex;
layout(set = 0, binding = 3) uniform sampler2D depthtex;
layout(set = 0, binding = 4) uniform sampler2D postex;

layout(set = 0, binding = 5) uniform Uniforms {
    int kernelSize;
    float radius;
    float bias;
    float intensity;
    mat4 projection;
    float THRESHOLD;
    float KNEE;
};

void main() {
    fragColor = vec4(texture(diftex,fUv).xyz, 1.0);
    depth = 0.0;
}