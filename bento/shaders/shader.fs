#version 330 core
uniform sampler2D tex;
in vec2 fragUV;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec4 fragNormalOut;
in vec3 fragNormal;
layout(location = 0) out float depth;
in vec4 fragUPos;
layout(location = 3) out vec4 fragPosOut;
in vec3 fragPos;
void main()
{
    vec3 textureColor = texture(tex, fragUV).xyz;
    fragColor = vec4(textureColor, 1.0);
    fragNormalOut = vec4((fragNormal / vec3(2.0)) + vec3(0.5), 1.0);
    depth = fragUPos.z;
    fragPosOut = vec4(fragPos, 1.0);
}