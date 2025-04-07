#version 330 core
layout(location = 0) in vec3 position;
out vec2 fUv;
layout(location = 2) in vec2 uv;
layout(location = 1) in vec3 normal;
void main()
{
    gl_Position = vec4(position, 1.0);
    fUv = vec2(uv.x, uv.y);
}