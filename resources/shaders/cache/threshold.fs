#version 330 core
uniform float threshold;
uniform sampler2D diftex;
in vec2 fUv;
layout(location = 1) out vec4 fragColor;
layout(location = 0) out float depth;
void main()
{
    vec2 uv = vec2(fUv.x, 1.0 - fUv.y);
    vec3 color = texture(diftex, uv).xyz;
    if ((((color.x + color.y) + color.z) / 3.0) > threshold)
    {
        fragColor = vec4(color, 1.0);
    }
    else
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    depth = 0.0;
}