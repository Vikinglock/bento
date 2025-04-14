#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragPos;

layout(set = 0, binding = 3) uniform Uniforms {
    mat4 lightProjection;
    mat4 model;
};

void main() {
    gl_Position = lightProjection * model  * vec4(position, 1.0);
    fragPos = vec3(lightProjection * model * vec4(position, 1.0));
}