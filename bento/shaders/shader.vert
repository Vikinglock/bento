#version 450
//this is so sad
#define MAX_LIGHTS 50

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec4 fragUPos;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out vec3 pos;

layout(set = 0, binding = 3) uniform Uniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 tpos;
};

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fragPos = vec3(model * vec4(position, 1.0));
    fragUPos = projection * view * model * vec4(position, 1.0);
    fragNormal = normal;
    fragUV.x = uv.x;
    fragUV.y = uv.y;
    viewPos = -view[3].xyz;
    pos = tpos;
    //viewDir = normalize(-view[2].xyz);//(vector twixt 0 and 1 (0,1,0 when looking up 1,0,0 forth 0,0,1 right))
}