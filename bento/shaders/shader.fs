#version 330 core
uniform sampler2D tex;
layout(location = 0) out vec4 fragColor;
in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPos;
in vec3 viewPos;
in vec3 viewDir;
void main()
{
    fragColor = vec4(texture(tex, fragUV).xyz * ((dot(fragNormal, vec3(0.0, 1.0, 0.0)) * 0.5) + 0.5), 1.0);
}