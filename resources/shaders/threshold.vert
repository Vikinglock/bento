#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 0) out vec2 fUv;
//layout(set = 0, binding = 3) uniform Uniforms {};

void main() {

    gl_Position = vec4(position, 1.0);
    fUv = vec2(uv.x,uv.y);

}