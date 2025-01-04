#version 330 core
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 tpos;
layout(location = 0) in vec3 position;
out vec3 fragPos;
out vec3 fragNormal;
layout(location = 1) in vec3 normal;
out vec2 fragUV;
layout(location = 2) in vec2 uv;
out vec3 viewPos;
out vec3 pos;
void main()
{
    gl_Position = ((projection * view) * model) * vec4(position, 1.0);
    fragPos = vec3((model * vec4(position, 1.0)).xyz);
    fragNormal = normal;
    fragUV.x = uv.x;
    fragUV.y = uv.y;
    viewPos = -view[3].xyz;
    pos = tpos;
}