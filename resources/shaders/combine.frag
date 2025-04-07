#version 450
layout(location = 0) out float depth;
layout(location = 1) out vec4 fragColor;
layout(location = 0) in vec2 fUv;
layout(set = 0, binding = 0) uniform sampler2D diftex;
layout(set = 0, binding = 1) uniform sampler2D blurtex;
layout(set = 0, binding = 4) uniform Uniforms {
    float strength;
    float exposure;
};

void main() {
    vec2 uv = vec2(fUv.x,1.0-fUv.y);
    vec4 color = texture(diftex, uv);
    vec4 blurredColor = texture(blurtex, uv);
    fragColor = (color + blurredColor * strength)*exposure;

    depth = 0.0;
}