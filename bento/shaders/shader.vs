#version 330 core
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
layout(location = 0) in vec3 position;
out vec3 fragPos;
out vec4 fragUPos;
out vec3 fragNormal;
layout(location = 1) in vec3 normal;
out vec2 fragUV;
layout(location = 2) in vec2 uv;
void main()
{
    gl_Position = ((projection * view) * model) * vec4(position, 1.0);
    fragPos = vec3((model * vec4(position, 1.0)).xyz);
    fragUPos = ((projection * view) * model) * vec4(position, 1.0);
    fragNormal = vec3((inverse(transpose(model)) * vec4(normal, 1.0)).xyz);
    fragUV.x = uv.x;
    fragUV.y = uv.y;
}