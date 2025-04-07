#version 330 core
uniform float strength;
uniform float exposure;
uniform sampler2D diftex;
uniform sampler2D blurtex;
in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;
void main()
{
    vec2 uv = vec2(fUv.x, 1.0 - fUv.y);
    vec4 color = texture(diftex, uv);
    vec4 blurredColor = texture(blurtex, uv);
    fragColor = (color + (blurredColor * strength)) * exposure;
    depth = 0.0;
}