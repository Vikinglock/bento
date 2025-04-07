#version 450
//this is so sad

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec4 fragUPos;

layout(set = 0, binding = 3) uniform Uniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fragPos = vec3(model * vec4(position, 1.0));
    fragUPos = projection * view * model * vec4(position, 1.0);
    fragNormal = vec3(inverse(transpose(model))*vec4(normal,1.0));//I SEE WHY THEY INVERSE IT NOW
    fragUV.x = uv.x;
    fragUV.y = uv.y;
}